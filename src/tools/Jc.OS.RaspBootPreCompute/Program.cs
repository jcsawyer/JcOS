using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

public static class PreCompute
{
    private const ulong MmioRemapStart = 0x1FC00000UL;
    private const ulong MmioRemapEndInclusive = 0x1FFFFFFFUL;
    private const int L3Entries = 8192;
    private const int NumL2Tables = 8;

    public static int Main(string[] args)
    {
        try
        {
            if (args.Length != 2)
            {
                Console.Error.WriteLine("usage: Jc.OS.RaspBootPreCompute <kernel8.elf> <board>");
                return 1;
            }

            string elf = args[0];
            string board = args[1];

            if (!File.Exists(elf))
            {
                Console.Error.WriteLine($"ELF not found: {elf}");
                return 1;
            }

            ulong physMmioStart = board switch
            {
                "bsp_rpi3" => 0x3F000000UL,
                "bsp_rpi4" => 0xFE000000UL,
                _ => throw new InvalidOperationException($"Unsupported board: {board}")
            };

            ulong dataVma = SectionVma(elf, ".data");
            ulong textVma = SectionVma(elf, ".text");

            ulong kernelTablesAddr = SymbolAddr(elf, "KERNEL_TABLES");
            ulong physArgAddr = SymbolAddr(elf, "PHYS_KERNEL_TABLES_BASE_ADDR");
            ulong codeStart = SymbolAddr(elf, "__code_start");
            ulong codeEndExclusive = SymbolAddr(elf, "__code_end_exclusive");

            string tempDir = Path.Combine(Path.GetTempPath(), "jcos-precompute-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(tempDir);

            try
            {
                string dataBin = Path.Combine(tempDir, "data.bin");
                string textBin = Path.Combine(tempDir, "text.bin");

                Run("aarch64-elf-objcopy", $"--dump-section .data={Quote(dataBin)} --dump-section .text={Quote(textBin)} {Quote(elf)}");

                byte[] dataBytes = File.ReadAllBytes(dataBin);
                byte[] textBytes = File.ReadAllBytes(textBin);

                byte[] tableBytes = BuildTranslationTables(kernelTablesAddr, codeStart, codeEndExclusive, physMmioStart);

                long dataOffset = checked((long)(kernelTablesAddr - dataVma));
                if (dataOffset < 0 || dataOffset + tableBytes.Length > dataBytes.Length)
                    throw new InvalidOperationException("table patch out of range");

                Buffer.BlockCopy(tableBytes, 0, dataBytes, (int)dataOffset, tableBytes.Length);

                ulong l3Bytes = (ulong)(NumL2Tables * L3Entries * 8);
                ulong physTablesBaseAddr = kernelTablesAddr + l3Bytes;

                long textOffset = checked((long)(physArgAddr - textVma));
                if (textOffset < 0 || textOffset + 8 > textBytes.Length)
                    throw new InvalidOperationException("arg patch out of range");

                byte[] physTablesBaseBytes = BitConverter.GetBytes(physTablesBaseAddr);
                if (!BitConverter.IsLittleEndian)
                    Array.Reverse(physTablesBaseBytes);

                Buffer.BlockCopy(physTablesBaseBytes, 0, textBytes, (int)textOffset, 8);

                File.WriteAllBytes(dataBin, dataBytes);
                File.WriteAllBytes(textBin, textBytes);

                Run("aarch64-elf-objcopy", $"--update-section .data={Quote(dataBin)} --update-section .text={Quote(textBin)} {Quote(elf)}");
            }
            finally
            {
                if (Directory.Exists(tempDir))
                    Directory.Delete(tempDir, true);
            }

            Console.WriteLine($"Patched precomputed translation tables into {elf}");
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.Message);
            return 1;
        }
    }

    private static byte[] BuildTranslationTables(ulong kernelTablesAddr, ulong codeStart, ulong codeEndExclusive, ulong physMmioStart)
    {
        using var ms = new MemoryStream();
        using var bw = new BinaryWriter(ms);

        ulong attrCode = AttrBits(isCache: true, readOnly: true, pxn: false);
        ulong attrDram = AttrBits(isCache: true, readOnly: false, pxn: true);
        ulong attrDev = AttrBits(isCache: false, readOnly: false, pxn: true);

        for (ulong l2 = 0; l2 < NumL2Tables; l2++)
        {
            for (ulong l3 = 0; l3 < L3Entries; l3++)
            {
                ulong virt = (l2 << 29) + (l3 << 16);
                ulong phys = virt;
                ulong attr = attrDram;

                if (virt >= codeStart && virt < codeEndExclusive)
                {
                    attr = attrCode;
                }
                else if (virt >= MmioRemapStart && virt <= MmioRemapEndInclusive)
                {
                    phys = physMmioStart + (virt - MmioRemapStart);
                    attr = attrDev;
                }

                ulong shifted = (phys >> 16) & 0xFFFFFFFFFFFUL;
                ulong pageDesc = (shifted << 16) | (1UL << 10) | 0b11UL | attr;
                bw.Write(pageDesc);
            }
        }

        for (ulong l2 = 0; l2 < NumL2Tables; l2++)
        {
            ulong lvl3Phys = kernelTablesAddr + (l2 * (ulong)L3Entries * 8UL);
            ulong shifted = (lvl3Phys >> 16) & 0xFFFFFFFFFFFUL;
            ulong tableDesc = (shifted << 16) | 0b11UL;
            bw.Write(tableDesc);
        }

        return ms.ToArray();
    }

    private static ulong AttrBits(bool isCache, bool readOnly, bool pxn)
    {
        ulong desc = 0;

        if (isCache)
            desc |= (0b11UL << 8) | (0b001UL << 2);
        else
            desc |= (0b010UL << 8) | (0b000UL << 2);

        if (readOnly)
            desc |= (0b10UL << 6);

        if (pxn)
            desc |= (1UL << 53);

        desc |= (1UL << 54);
        return desc;
    }

    private static ulong SectionVma(string elf, string sectionName)
    {
        string output = Run("aarch64-elf-readelf", $"-W -S {Quote(elf)}");

        foreach (string line in output.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            Match m = Regex.Match(line,
                @"^\s*\[\s*\d+\]\s+(?<name>\S+)\s+\S+\s+(?<addr>[0-9A-Fa-f]+)");

            if (!m.Success)
                continue;

            if (m.Groups["name"].Value == sectionName)
                return ulong.Parse(m.Groups["addr"].Value, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
        }

        throw new InvalidOperationException($"Missing section: {sectionName}");
    }

    private static ulong SymbolAddr(string elf, string symbolName)
    {
        string output = Run("aarch64-elf-nm", $"-n {Quote(elf)}");

        foreach (string line in output.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 3)
                continue;

            if (parts[2] == symbolName)
                return ParseNum(parts[0]);
        }

        throw new InvalidOperationException($"Missing symbol: {symbolName}");
    }

    private static ulong ParseNum(string value)
    {
        string trimmed = value.Trim();
        if (trimmed.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            return ulong.Parse(trimmed[2..], NumberStyles.HexNumber, CultureInfo.InvariantCulture);

        return ulong.Parse(trimmed, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
    }

    private static string Run(string fileName, string arguments)
    {
        var psi = new ProcessStartInfo(fileName, arguments)
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };

        using var process = Process.Start(psi) ?? throw new InvalidOperationException($"Failed to start: {fileName}");

        string stdout = process.StandardOutput.ReadToEnd();
        string stderr = process.StandardError.ReadToEnd();

        process.WaitForExit();

        if (process.ExitCode != 0)
        {
            var sb = new StringBuilder();
            sb.AppendLine($"Command failed: {fileName} {arguments}");
            if (!string.IsNullOrWhiteSpace(stderr))
                sb.AppendLine(stderr.TrimEnd());
            throw new InvalidOperationException(sb.ToString().TrimEnd());
        }

        return stdout;
    }

    private static string Quote(string value)
    {
        return $"\"{value.Replace("\"", "\\\"")}" + "\"";
    }
}
