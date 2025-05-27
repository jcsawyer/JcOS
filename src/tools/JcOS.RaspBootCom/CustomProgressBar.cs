using System.Diagnostics;

namespace JcOS.RaspBootCom;

class CustomProgressBar
{
    private readonly int _totalKiB;
    private readonly Stopwatch _timer;
    private readonly string _processName;
    private int _currentKiB;

    public CustomProgressBar(int totalKiB, string processName)
    {
        _totalKiB = totalKiB;
        _timer = new Stopwatch();
        _processName = processName;
    }

    public void Start()
    {
        _currentKiB = 0;
        _timer.Restart();
        Render();
    }

    public void Report(int kiB)
    {
        _currentKiB = Math.Min(kiB, _totalKiB);
        Render();
    }

    public void Finish()
    {
        _timer.Stop();
        _currentKiB = _totalKiB;
        Render();
        Console.WriteLine();
    }

    private void Render()
    {
        Console.CursorLeft = 0;

        int totalBlocks = 35;
        int position = (int)((_currentKiB / (double)_totalKiB) * totalBlocks);
        position = Math.Min(position, totalBlocks - 1);

        string bar = "";

        for (int i = 0; i < totalBlocks; i++)
        {
            if (i == position)
                bar += "🍎";
            else
                bar += (i < position ? "=" : "-");
        }

        double percent = (_currentKiB / (double)_totalKiB) * 100;
        double elapsedSec = _timer.Elapsed.TotalSeconds;
        string speed = elapsedSec > 0
            ? $"{(_currentKiB / elapsedSec):0} KiB/s"
            : "0 KiB/s";
        string time = _timer.Elapsed.ToString(@"hh\:mm\:ss");

        Console.Write($"{_processName} {_totalKiB} KiB {bar} {percent,3:0}% {speed} Time: {time}");
    }
}