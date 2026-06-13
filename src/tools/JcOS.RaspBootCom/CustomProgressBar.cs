using System.Diagnostics;

namespace JcOS.RaspBootCom;

class CustomProgressBar
{
    private readonly int _totalBytes;
    private readonly Stopwatch _timer;
    private readonly string _processName;
    private int _currentBytes;

    public CustomProgressBar(int totalBytes, string processName)
    {
        _totalBytes = totalBytes;
        _timer = new Stopwatch();
        _processName = processName;
    }

    public void Start()
    {
        _currentBytes = 0;
        _timer.Restart();
        Render();
    }

    public void Report(int bytes)
    {
        _currentBytes = Math.Min(bytes, _totalBytes);
        Render();
    }

    public void Finish()
    {
        _timer.Stop();
        _currentBytes = _totalBytes;
        Render();
        Console.WriteLine();
    }

    private void Render()
    {
        Console.CursorLeft = 0;

        int totalBlocks = 35;
        int position = (int)((_currentBytes / (double)_totalBytes) * totalBlocks);
        position = Math.Min(position, totalBlocks - 1);

        string bar = "";

        for (int i = 0; i < totalBlocks; i++)
        {
            if (i == position)
                bar += "🍎";
            else
                bar += (i < position ? "=" : "-");
        }

        double percent = (_currentBytes / (double)_totalBytes) * 100;
        double elapsedSec = _timer.Elapsed.TotalSeconds;
        double currentKiB = _currentBytes / 1024.0;
        double totalKiB = _totalBytes / 1024.0;
        string speed = elapsedSec > 0
            ? $"{(currentKiB / elapsedSec):0.0} KiB/s"
            : "0 KiB/s";
        string time = _timer.Elapsed.ToString(@"hh\:mm\:ss");

        Console.Write($"{_processName} {totalKiB:0.0} KiB {bar} {percent,3:0}% {speed} Time: {time}");
    }
}
