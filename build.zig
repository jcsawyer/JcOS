const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{ .default_target = .{
        .abi = .none,
        .os_tag = .freestanding,
        .cpu_arch = .aarch64,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_a53 },
    } });

    const kernel_cpp = b.addStaticLibrary(.{
        .name = "kernel_cpp",
        .target = target,
        .optimize = .ReleaseSafe,
    });

    kernel_cpp.addIncludePath(b.path("src"));
    kernel_cpp.addIncludePath(b.path("src/std"));
    kernel_cpp.addIncludePath(b.path("src/_arch"));
    kernel_cpp.addIncludePath(b.path("src/bsp"));
    kernel_cpp.addIncludePath(b.path("src/bsp/raspberrypi"));
    kernel_cpp.addIncludePath(b.path("src/console"));

    kernel_cpp.addCSourceFiles(.{
        .flags = &.{
            "-ffreestanding",
            "-O2",
            "-Wall",
            "-Wextra",
            "-fno-exceptions",
            "-fno-rtti",
        },
        .files = &.{
            "src/main.cpp",
            "src/console.cpp",
            "src/driver.cpp",
            "src/std/printf.c",
            "src/_arch/aarch64/cpu/boot.cpp",
            "src/bsp/device_driver/bcm/bcm2xxx_gpio.cpp",
            "src/bsp/device_driver/bcm/bcm2xxx_pl011_uart.cpp",
            //"src/bsp/raspberrypi/qemu_console.cpp",
            "src/bsp/raspberrypi/raspi_driver.cpp",
            "src/console/null_console.cpp",
        },
    });

    b.installArtifact(kernel_cpp);

    const kernel = b.addExecutable(.{
        .name = "kernel",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = .ReleaseSafe,
        .linkage = .static,
        .code_model = .small,
    });

    kernel.linkLibrary(kernel_cpp);
    kernel.setLinkerScript(b.path("src/bsp/raspberrypi/kernel.ld"));

    const board = b.option([]const u8, "board", "The board to target.") orelse "bsp_rpi3";
    const options = b.addOptions();
    options.addOption([]const u8, "board", board);

    kernel.root_module.addOptions("config", options);

    kernel.defineCMacro("RASPI_BOARD", board);

    b.installArtifact(kernel);

    const bin = b.addObjCopy(kernel.getEmittedBin(), .{ .format = .elf });
    const bin_step = b.addInstallBinFile(bin.getOutput(), "kernel8.img");
    b.default_step.dependOn(&bin_step.step);

    const qemu = b.addSystemCommand(&[_][]const u8{ "qemu-system-aarch64", "-M", "raspi3b", "-serial", "stdio", "-display", "none", "-kernel", b.getInstallPath(bin_step.dir, bin_step.dest_rel_path) });
    qemu.step.dependOn(&bin_step.step);
    const qemu_step = b.step("qemu", "Run kernel in QEMU.");
    qemu_step.dependOn(&qemu.step);
}
