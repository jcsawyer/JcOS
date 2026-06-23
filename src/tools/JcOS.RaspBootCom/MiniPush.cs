using System.Text;
using JcOS.RaspBootCom;

public class ProtocolError : Exception
{
    public ProtocolError(string message = "Protocol Error") : base(message)
    {
    }
}

public class MiniPush : MiniTerm
{
    private readonly string _payloadPath;
    private byte[] _payloadData = [];
    private int _payloadSize;
    private int _transferAttempt;
    private const int HostWriteChunkSize = 8192;
    private const int HandshakeTimeoutSeconds = 10;
    private const int MinimumTransferTimeoutSeconds = 30;
    private const int TransferTimeoutPaddingSeconds = 15;
    private const int AssumedMinimumTransferRateBytesPerSecond = 8 * 1024;

    public MiniPush(string serialName, string payloadPath) : base(serialName)
    {
        _nameShort = "MP";
        _payloadPath = payloadPath;
    }

    private static bool IsPrintable(byte value)
    {
        return value is >= 0x20 and <= 0x7E || value is (byte)'\r' or (byte)'\n' or (byte)'\t';
    }

    private void DrainSerialInput()
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        while (true)
        {
            try
            {
                if (_targetSerial.BytesToRead == 0)
                    break;

                _targetSerial.ReadByte();
            }
            catch (TimeoutException)
            {
                break;
            }
        }
    }

    private void PrintConsoleByte(byte value)
    {
        if (value == (byte)'\r')
        {
            Console.Write("\r\n");
            return;
        }

        if (value == (byte)'\n')
            return;

        if (IsPrintable(value))
            Console.Write((char)value);
    }

    private void WaitForPayloadRequest(CancellationToken token)
    {
        var buffer = new byte[4096];
        int count = 0;

        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        while (true)
        {
            if (token.IsCancellationRequested)
                throw new TimeoutException();

            int bytesRead;
            try
            {
                bytesRead = _targetSerial.Read(buffer, 0, buffer.Length);
            }
            catch (TimeoutException)
            {
                continue;
            }

            for (int i = 0; i < bytesRead; i++)
            {
                if (buffer[i] == 0x03)
                {
                    // Wait for 3 consecutive 0x03 bytes
                    count++;
                    if (count == 3)
                        return;
                }
                else
                {
                    count = 0;
                    PrintConsoleByte(buffer[i]);
                }
            }
        }
    }

    private void LoadPayload()
    {
        if (!File.Exists(_payloadPath))
            throw new FileNotFoundException($"Payload image not found: {_payloadPath}", _payloadPath);

        _payloadData = File.ReadAllBytes(_payloadPath);
        _payloadSize = _payloadData.Length;
    }

    private byte[] ReadExact(int length, CancellationToken token)
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        var buffer = new byte[length];
        int offset = 0;

        while (offset < length)
        {
            token.ThrowIfCancellationRequested();

            try
            {
                int read = _targetSerial.Read(buffer, offset, length - offset);
                if (read == 0)
                    continue;

                offset += read;
            }
            catch (TimeoutException)
            {
            }
        }

        return buffer;
    }

    private byte ReadControlByte(
        CancellationToken token,
        ReadOnlySpan<byte> echoedBytes,
        bool tolerateTrailingEchoBytes = false,
        params byte[] allowedValues)
    {
        var echoIndex = 0;

        while (true)
        {
            var response = ReadExact(1, token)[0];

            // Ignore extra ready-handshake bytes that were already in flight
            // when the host transitioned into the size/payload phases.
            if (response == 0x03)
                continue;

            // Some USB serial paths echo outbound writes back to RX. Ignore
            // those bytes while waiting for protocol control responses.
            if (echoIndex < echoedBytes.Length && response == echoedBytes[echoIndex])
            {
                echoIndex++;
                continue;
            }

            if (allowedValues.Contains(response))
                return response;

            // Once we've transitioned from payload to control bytes, some
            // adapters still leak a few stray echoed payload bytes. Skip them
            // rather than treating them as a hard protocol fault.
            if (tolerateTrailingEchoBytes && echoIndex < echoedBytes.Length)
                continue;

            throw new ProtocolError($"Unexpected control byte 0x{response:X2}");
        }
    }

    private bool SendSize(CancellationToken token)
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        Console.WriteLine($"[{_nameShort}] 📏 Sending payload size: {_payloadSize} bytes");
        var sizeBytes = BitConverter.GetBytes(_payloadSize);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(sizeBytes);

        _targetSerial.Write(sizeBytes, 0, 4);

        var first = ReadControlByte(token, sizeBytes, false, (byte)'O', (byte)'S');
        var second = ReadControlByte(token, [], false, (byte)'K', (byte)'E');
        if (first == 'O' && second == 'K')
            return true;

        if (first == 'S' && second == 'E')
            return false;

        throw new ProtocolError($"Unexpected size response '{(char)first}{(char)second}'");
    }

    private void SendPayload(CancellationToken token)
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        Console.WriteLine($"[{_nameShort}] 🚀 Streaming payload");
        var pb = new CustomProgressBar(_payloadSize, $"[{_nameShort}] ⏩ Pushing");
        pb.Start();
        var written = 0;

        var initialReady = ReadControlByte(token, [], false, (byte)'C');
        if (initialReady != (byte)'C')
            throw new ProtocolError($"Unexpected initial payload ready 0x{initialReady:X2}");

        while (written < _payloadSize)
        {
            token.ThrowIfCancellationRequested();

            var chunkSize = Math.Min(HostWriteChunkSize, _payloadSize - written);
            _targetSerial.Write(_payloadData, written, chunkSize);
            pb.Report(written + chunkSize);
            var ack = ReadControlByte(
                token,
                _payloadData.AsSpan(written, chunkSize),
                true,
                (byte)'C');
            if (ack != (byte)'C')
                throw new ProtocolError($"Unexpected payload ack 0x{ack:X2}");

            written += chunkSize;
        }

        pb.Finish();
    }

    private TimeSpan ComputeAttemptTimeout()
    {
        var estimatedTransferSeconds =
            (_payloadSize + AssumedMinimumTransferRateBytesPerSecond - 1) /
            AssumedMinimumTransferRateBytesPerSecond;
        var transferTimeoutSeconds = Math.Max(
            MinimumTransferTimeoutSeconds,
            estimatedTransferSeconds + TransferTimeoutPaddingSeconds);

        return TimeSpan.FromSeconds(HandshakeTimeoutSeconds + transferTimeoutSeconds);
    }

    private bool TryTransferOnce()
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        _transferAttempt++;
        Console.WriteLine($"[{_nameShort}] 🔄 Waiting for chainloader (attempt {_transferAttempt})");
        DrainSerialInput();

        using var attemptCts = new CancellationTokenSource(ComputeAttemptTimeout());

        try
        {
            WaitForPayloadRequest(attemptCts.Token);
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] 🤝 Chainloader ready");
            if (!SendSize(attemptCts.Token))
            {
                Console.WriteLine($"[{_nameShort}] 📦 Chainloader rejected payload size, waiting for next request");
                return false;
            }

            SendPayload(attemptCts.Token);
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] ✅ Payload transferred");
            return true;
        }
        catch (TimeoutException)
        {
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] ⏳ No complete handshake yet, retrying on the same serial session");
            return false;
        }
        catch (OperationCanceledException)
        {
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] ⏳ Transfer attempt timed out, retrying on the same serial session");
            return false;
        }
        catch (ProtocolError ex)
        {
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] ⚠️ Protocol mismatch: {ex.Message}");
            return false;
        }
    }

    protected override void HandleReconnect(Exception? ex)
    {
        ConnectionReset();

        Console.WriteLine();
        Console.WriteLine(
            $"[{_nameShort}] ⚡ Serial connection lost. Waiting for the target serial device to come back.");
        while (SerialConnected())
        {
            Thread.Sleep(1000);
        }
    }

    public override void Run()
    {
        try
        {
            OpenSerial();
            LoadPayload();

            while (!TryTransferOnce())
            {
            }

            Terminal();
        }
        catch (FileNotFoundException ex)
        {
            ConnectionReset();
            Console.WriteLine();
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"[{_nameShort}] ⚠️ Payload image missing: {ex.FileName ?? _payloadPath}");
            Console.WriteLine($"[{_nameShort}] Build JcOS first so the image exists before starting MiniPush.");
            Console.ResetColor();
        }
        catch (IOException)
        {
            HandleReconnect(null);
            Run();
        }
        catch (ProtocolError)
        {
            HandleReconnect(null);
            Run();
        }
        catch (TimeoutException)
        {
            HandleReconnect(null);
            Run();
        }
        catch (Exception e)
        {
            HandleUnexpected(e);
        }
        finally
        {
            ConnectionReset();
            Console.WriteLine();
            Console.WriteLine($"[{_nameShort}] Bye 👋");
        }
    }
}
