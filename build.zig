const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{ .default_target = .{
        .abi = .eabihf,
        .os_tag = .freestanding,
        .cpu_arch = .aarch64,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_a53 },
    } });

    const kernel = b.addExecutable(.{
        .name = "kernel",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = .ReleaseSafe,
        .linkage = .static,
        .code_model = .small,
    });

    kernel.addIncludePath(b.path("src/std"));
    kernel.addCSourceFile(.{ .file = b.path("src/std/printf.c") });

    kernel.setLinkerScript(b.path("src/bsp/raspberrypi/kernel.ld"));

    const board = b.option([]const u8, "board", "The board to target.") orelse "bsp_rpi3";
    const options = b.addOptions();
    options.addOption([]const u8, "board", board);

    kernel.root_module.addOptions("config", options);

    b.installArtifact(kernel);

    const bin = b.addObjCopy(kernel.getEmittedBin(), .{ .format = .elf });
    const bin_step = b.addInstallBinFile(bin.getOutput(), "kernel8.img");
    b.default_step.dependOn(&bin_step.step);

    const qemu = b.addSystemCommand(&[_][]const u8{ "qemu-system-aarch64", "-M", "raspi3b", "-serial", "stdio", "-display", "none", "-kernel", b.getInstallPath(bin_step.dir, bin_step.dest_rel_path) });
    qemu.step.dependOn(&bin_step.step);
    const qemu_step = b.step("qemu", "Run kernel in QEMU.");
    qemu_step.dependOn(&qemu.step);
}
