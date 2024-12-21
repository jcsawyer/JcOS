const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{ .default_target = .{
        .abi = .eabihf,
        .os_tag = .freestanding,
        .cpu_arch = .aarch64,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_a53 },
    } });
    const optimize = b.standardOptimizeOption(.{});

    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    kernel.setLinkerScript(b.path("src/linker.ld"));
    b.installArtifact(kernel);

    const bin = b.addObjCopy(kernel.getEmittedBin(), .{ .format = .bin });
    const bin_step = b.addInstallBinFile(bin.getOutput(), "kernel.img");
    b.default_step.dependOn(&bin_step.step);

    const qemu = b.addSystemCommand(&[_][]const u8{ "qemu-system-aarch64", "-M", "raspi3b", "-display", "none", "-serial", "null", "-serial", "stdio", "-kernel", b.getInstallPath(bin_step.dir, bin_step.dest_rel_path) });
    qemu.step.dependOn(&bin_step.step);
    const qemu_step = b.step("qemu", "Run kernel in QEMU.");
    qemu_step.dependOn(&qemu.step);
}
