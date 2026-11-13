# Security Model — Initial Baseline

## Trust zones

1. **Firmware zone:** UEFI firmware and Boot Services are trusted only during the boot phase required to load the image and gather boot data.
2. **Kernel zone:** after ExitBootServices, kernel code owns its execution path but does not yet provide isolation boundaries. All current kernel code is effectively privileged.
3. **Future service zone:** identity, PKI, policy evaluation, update verification, privacy governance and audit services must be separated from low-level memory and CPU primitives.

## Bootstrap rules

- Never persist private keys, tokens, passwords or signing material in the repository.
- Never implement home-grown cryptographic algorithms.
- Do not treat a successful UEFI handoff as proof of Secure Boot verification; signature enforcement must be separately specified and tested.
- Treat memory-map pointers, sizes and descriptor strides as externally supplied input.
- Do not reclaim boot-services memory until every live boot allocation and the active kernel image have explicit ownership records.
- Do not enable interrupts before a valid IDT and interrupt-controller setup exist.
- Keep diagnostic output free of credentials and secrets.

## PKI and governance integration

PKI is a platform security service, not a kernel convenience library. The SPEC-003 workstream, including the accepted variants for privacy/data governance, developer integration, release distribution, operations and incident response, must be translated into reviewable interfaces, lifecycle rules, threat models and test requirements before implementation. The exact approved variant details must be preserved from their decision records; this bootstrap does not invent missing policy text.
