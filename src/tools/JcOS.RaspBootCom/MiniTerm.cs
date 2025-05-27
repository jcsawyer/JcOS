using System.IO.Ports;

public class ConnectionError : Exception
{
    public ConnectionError(string message = "Connection Error") : base(message)
    {
    }
}

public class MiniTerm
{
    protected string _nameShort = "MT";
    protected string _targetSerialName;
    protected SerialPort _targetSerial;

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

            _targetSerial = new SerialPort(_targetSerialName, 921_600, Parity.None, 8, StopBits.One);
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

        var receiveThread = new Thread(() =>
        {
            try
            {
                while (true)
                {
                    int b = _targetSerial.ReadByte();
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
            }
            catch
            {
                throw new ConnectionError();
            }
        });

        receiveThread.IsBackground = true;
        receiveThread.Start();

        while (true)
        {
            var key = Console.ReadKey(intercept: true);
            if (key.Key == ConsoleKey.C && key.Modifiers.HasFlag(ConsoleModifiers.Control))
            {
                receiveThread.Interrupt();
                break;
            }

            _targetSerial.Write(key.KeyChar.ToString());
        }
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

    protected virtual void HandleReconnect(Exception _)
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

    public static void Main(string[] args)
    {
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine("\nMiniterm 1.0\n");
        Console.ResetColor();

        Console.CancelKeyPress += (_, _) => Environment.Exit(0);

        if (args.Length < 1)
        {
            Console.WriteLine("Usage: MiniTerm <serialPortPath>");
            return;
        }

        var miniTerm = new MiniTerm(args[0]);
        miniTerm.Run();
    }
}