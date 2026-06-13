Console.ForegroundColor = ConsoleColor.Cyan;
Console.WriteLine("\nMinipush 1.0\n");
Console.ResetColor();

Console.CancelKeyPress += (_, _) => Environment.Exit(0);

if (args.Length < 2)
{
    Console.WriteLine("Usage: MiniPush <serialPort> <payloadFile>");
    return;
}

if (!File.Exists(args[1]))
{
    Console.ForegroundColor = ConsoleColor.Yellow;
    Console.WriteLine($"[MP] ⚠️ Payload image missing: {args[1]}");
    Console.WriteLine("[MP] Build JcOS first so the kernel image exists before attempting transfer.");
    Console.ResetColor();
    return;
}

var miniPush = new MiniPush(args[0], args[1]);
miniPush.Run();
