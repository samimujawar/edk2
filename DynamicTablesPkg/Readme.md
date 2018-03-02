Dynamic Tables Framework
------------------------

To reduce the amount of effort required in porting firmware to new
platforms, we propose this "Dynamic Tables" framework.  The aim is
to provide an example implementation capable of generating the
firmware tables from an external source.  This is potentially a
management node, either local or remote, or, where suitable, a file
that might be generated from the system construction.  This initial
"proof of concept" release does not fully implement that - the
configuration is held in local UEFI modules.

Branch Owners
-------------
 Evan Lloyd <evan.lloyd at arm.com> \
 Sami Mujawar <sami.mujawar at arm.com>

Feature Summary
---------------
The dynamic tables framework is designed to generate standardised
firmware tables that describe the hardware information at
run-time. A goal of standardised firmware is to have a common
firmware for a platform capable of booting both Windows and Linux
operating systems.

Traditionally the firmware tables are handcrafted using ACPI
Source Language (ASL), Table Definition Language (TDL) and
C-code. This approach can be error prone and involves time
consuming debugging. In addition, it may be desirable to configure
platform hardware at runtime such as: configuring the number of
cores available for use by the OS, or turning SoC features ON or
OFF.

The dynamic tables framework simplifies this by providing a set
of standard table generators, that are implemented as libraries.
These generators query a platform specific component, the
'Configuration Manager', to collate the information required
for generating the tables at run-time.

The framework also provides the ability to implement custom/OEM
generators; thereby facilitating support for custom tables. The
custom generators can also utilize the existing standard generators
and override any functionality if needed.

The framework currently implements a set of standard ACPI table
generators for ARM architecture, that can generate Server Base Boot
Requirement (SBBR) compliant tables. Although, the set of standard
generators implement the functionality required for ARM architecture;
the framework is extensible, and support for other architectures can
be added easily.

The framework currently supports the following table generators for ARM:
* DBG2 - Debug Port Table 2
* DSDT - Differentiated system description table. This is essentially
         a RAW table generator.
* FADT - Fixed ACPI Description Table
* GTDT - Generic Timer Description Table
* IORT - IO Remapping Table
* MADT - Multiple APIC Description Table
* MCFG - PCI Express memory mapped configuration space base address
         Description Table
* SPCR - Serial Port Console Redirection Table
* SSDT - Secondary System Description Table. This is essentially
         a RAW table generator.

Roadmap
-------
The current implementation of the Configuration Manager populates the
platform information statically as a C structure. Further enhancements
to introduce runtime loading of platform information from a platform
information file is planned.

Also support for generating SMBIOS tables is planned and will be added
subsequently.

Related Modules
---------------

### edk2-platforms
The *devel-dynamictables* branch in the **edk2-platform** repository contains
the Configuration Manager implementation (the platform specific component)
for Juno and Fixed Virtual Platform models.

### ACPICA iASL compiler
The RAW table generator, used to process the DSDT/SSDT files depends on
the iASL compiler to convert the DSDT/SSDT ASL files to a C array containing
the hex AML code. The current implementation of the iASL compiler does not
support generation of a C header file suitable for including from a C source
file.

A patch ***'Add support for hex AML C header file generation'***, to enable
this support has been submitted to the ACPICA source repository.

Related Links
--------------

<https://github.com/tianocore/edk2-platforms.git>

<https://github.com/acpica/acpica.git>

Build Instructions
------------------
To enable Dynamic tables framework the *'DYNAMIC_TABLES_FRAMEWORK'*
option must be defined. This can be passed as a command line
parameter to the edk2 build system.

Example:

>build -a AARCH64 -p Platform\ARM\JunoPkg\ArmJuno.dsc -t GCC5
  **-D DYNAMIC_TABLES_FRAMEWORK**

Prerequisites
-------------
ACPICA iASL compiler with support for generating a C header file.

Documentation
-------------
A description document is in preparation, and should be available in the
near future.

Miscellaneous
-------------
