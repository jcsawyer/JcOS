using System.IO.Ports;

public class ConnectionError : Exception
{
    public ConnectionError(string message = "Connection Error") : base(message)
    {
    }
}

public class MiniTerm
{
    protected const int BaudRate = 921_600;
    protected const int PostPayloadBaudRate = 115_200;
    protected string _nameShort = "MT";
    protected string _targetSerialName;
    protected SerialPort? _targetSerial;

    public MiniTerm(string serialName)
    {
        _targetSerialName = serialName;
    }

    protected bool SerialConnected()
    {
        return File.Exists(_targetSerialName);
    }

    protected void WaitForSerial()
    {
        if (SerialConnected()) return;

        Console.WriteLine($"[{_nameShort}] ⏳ Waiting for {_targetSerialName}");
        while (!SerialConnected())
        {
            Thread.Sleep(1000);
        }
    }

    protected void OpenSerial()
    {
        try
        {
            WaitForSerial();

            _targetSerial = new SerialPort(_targetSerialName, BaudRate, Parity.None, 8, StopBits.One);
            _targetSerial.ReadTimeout = 250;
            _targetSerial.WriteTimeout = SerialPort.InfiniteTimeout;
            _targetSerial.Open();

            Console.WriteLine($"[{_nameShort}] ✅ Serial connected");
        }
        catch (UnauthorizedAccessException ex)
        {
            Console.WriteLine($"[{_nameShort}] 🚫 {ex.Message} - Maybe try as Administrator");
            Environment.Exit(1);
        }
    }

    protected void Terminal()
    {
        Console.TreatControlCAsInput = true;
        var disconnected = false;
        Exception? receiveError = null;

        var receiveThread = new Thread(() =>
        {
            while (!disconnected)
            {
                try
                {
                    int b = _targetSerial!.ReadByte();
                    if (b == -1)
                        throw new ConnectionError();

                    char c = (char)b;

                    if (c == 0x08 || c == 0x7F && Console.CursorLeft > 0) // Backspace or Delete
                    {
                        Console.Write("\b \b");
                    }
                    else if (c == '\r')
                    {
                        Console.Write("\r\n");
                    }
                    else
                    {
                        Console.Write(c);
                    }
                }
                catch (TimeoutException)
                {
                    continue;
                }
                catch (Exception ex)
                {
                    receiveError = ex;
                    disconnected = true;
                }
            }
        });

        receiveThread.IsBackground = true;
        receiveThread.Start();

        while (!disconnected)
        {
            if (!Console.KeyAvailable)
            {
                Thread.Sleep(25);
                continue;
            }

            var key = Console.ReadKey(intercept: true);
            if (key.Key == ConsoleKey.C && key.Modifiers.HasFlag(ConsoleModifiers.Control))
            {
                disconnected = true;
                break;
            }

            _targetSerial!.Write(key.KeyChar.ToString());
        }

        receiveThread.Join(500);

        if (receiveError is not null)
            throw new ConnectionError(receiveError.Message);
    }

    protected void SwitchToPostPayloadBaudRate()
    {
        if (_targetSerial == null)
            throw new InvalidOperationException("Serial port not initialized");

        _targetSerial.BaseStream.Flush();
        _targetSerial.BaudRate = PostPayloadBaudRate;
    }

    protected void ConnectionReset()
    {
        try
        {
            _targetSerial?.Close();
            _targetSerial = null;
        }
        catch
        {
        }

        Console.TreatControlCAsInput = false;
    }

    protected virtual void HandleReconnect(Exception? _)
    {
        ConnectionReset();

        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine($"[{_nameShort}] ⚡ Connection Error: Reinsert the USB serial again");
        Console.ResetColor();
    }

    protected virtual void HandleUnexpected(Exception error)
    {
        ConnectionReset();

        Console.WriteLine();
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine($"[{_nameShort}] ⚡ Unexpected Error: {error}");
        Console.ResetColor();
    }

    public virtual void Run()
    {
        try
        {
            OpenSerial();
            Terminal();
        }
        catch (ConnectionError)
        {
            HandleReconnect(null);
            Run();
        }
        catch (IOException)
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
