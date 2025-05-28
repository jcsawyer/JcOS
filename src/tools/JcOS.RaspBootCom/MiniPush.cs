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
    private readonly string _nameShort = "MP";
    private readonly string _payloadPath;
    private byte[] _payloadData;
    private int _payloadSize;

    public MiniPush(string serialName, string payloadPath) : base(serialName)
    {
        _payloadPath = payloadPath;
    }

    private void WaitForPayloadRequest()
    {
        Console.WriteLine($"[{_nameShort}] 🔌 Please power the target now");

        var buffer = new byte[4096];
        int count = 0;

        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        using var timeoutCts = new CancellationTokenSource(TimeSpan.FromSeconds(10));
        var token = timeoutCts.Token;

        while (true)
        {
            if (token.IsCancellationRequested)
                throw new TimeoutException();

            int bytesRead = _targetSerial.Read(buffer, 0, buffer.Length);
            if (bytesRead == 0)
                continue;

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
                    Console.Write((char)buffer[i]);
                }
            }
        }
    }

    private void LoadPayload()
    {
        _payloadData = File.ReadAllBytes(_payloadPath);
        _payloadSize = _payloadData.Length;
    }

    private void SendSize()
    {
        var sizeBytes = BitConverter.GetBytes(_payloadSize);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(sizeBytes);

        _targetSerial.Write(sizeBytes, 0, 4);

        var response = new byte[2];
        int read = _targetSerial.Read(response, 0, 2);
        if (read != 2 || Encoding.ASCII.GetString(response) != "OK")
            throw new ProtocolError("Expected 'OK' after size send");
    }

    private void SendPayload()
    {
        var pb = new CustomProgressBar(_payloadSize, $"[{_nameShort}] ⏩ Pushing");

        int progress = 0;

        while (progress < _payloadSize)
        {
            int chunkSize = Math.Min(512, _payloadSize - progress);
            _targetSerial.Write(_payloadData, progress, chunkSize);
            progress += chunkSize;
            pb.Report(progress);
        }

        pb.Finish();
    }

    protected override void HandleReconnect(Exception ex)
    {
        ConnectionReset();

        Console.WriteLine();
        Console.WriteLine(
            $"[{_nameShort}] ⚡ Connection or protocol error: Remove power and USB serial. Reinsert serial first, then power.");
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
            WaitForPayloadRequest();
            LoadPayload();
            SendSize();
            SendPayload();
            Terminal();
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

    public static void Main(string[] args)
    {
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine("\nMinipush 1.0\n");
        Console.ResetColor();

        Console.CancelKeyPress += (_, _) => Environment.Exit(0);

        if (args.Length < 2)
        {
            Console.WriteLine("Usage: MiniPush <serialPort> <payloadFile>");
            return;
        }

        var miniPush = new MiniPush(args[0], args[1]);
        miniPush.Run();
    }
}