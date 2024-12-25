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
        .code_model = .tiny,
        .linkage = .static,
        .pic = false,
        .omit_frame_pointer = false,
    });
    kernel.addIncludePath(b.path("src"));
    kernel.setLinkerScript(b.path("src/linker.ld"));

    // Disable features that are problematic in kernel space.
    kernel.root_module.red_zone = false;
    kernel.root_module.stack_check = false;
    kernel.root_module.stack_protector = false;
    kernel.want_lto = false;
    // Delete unused sections to reduce the kernel size.
    kernel.link_function_sections = true;
    kernel.link_data_sections = true;
    kernel.link_gc_sections = true;
    // Force the page size to 4 KiB to prevent binary bloat.
    kernel.link_z_max_page_size = 0x1000;

    b.installArtifact(kernel);

    const bin = b.addObjCopy(kernel.getEmittedBin(), .{ .format = .bin });
    const bin_step = b.addInstallBinFile(bin.getOutput(), "kernel.img");
    b.default_step.dependOn(&bin_step.step);

    const qemu = b.addSystemCommand(&[_][]const u8{ "qemu-system-aarch64", "-M", "raspi3b", "-serial", "stdio", "-serial", "null", "-kernel", b.getInstallPath(bin_step.dir, bin_step.dest_rel_path) });
    qemu.step.dependOn(&bin_step.step);
    const qemu_step = b.step("qemu", "Run kernel in QEMU.");
    qemu_step.dependOn(&qemu.step);
}
