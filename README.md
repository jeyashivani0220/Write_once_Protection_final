# Write-Once Protection for NvM Blocks

## 1. Project Overview

The Write-Once Protection project implements a mechanism to prevent repeated modification of selected Non-Volatile Memory (NvM) blocks in an embedded system. It allows the first valid write operation to a protected block and rejects subsequent write requests once the block has been successfully written and locked.

The project is developed using Embedded C on the NXP S32K144 microcontroller in S32 Design Studio (S32DS). It uses block-level protection flags and lock-status tracking to control write access and return appropriate request-status values.

## 2. Objectives

- Implement write-once protection for configurable memory blocks.
- Allow the first successful write operation to a protected block.
- Prevent subsequent writes to locked blocks.
- Validate block IDs before processing write requests.
- Maintain individual protection and lock status for each block.
- Improve the reliability and integrity of stored configuration data.

## 3. Architecture Diagram

```text
       Application Layer
               |
               v
        NvM Write Request
               |
               v
      Block ID Validation
               |
               v
    Write Protection Check
               |
               v
      Is Block Locked?
          /         \
        Yes          No
         |            |
         v            v
   Reject Write    Perform Write
         |            |
         v            v
 NVM_REQ_NOT_OK   Write Successful?
                      /    \
                    Yes     No
                     |       |
                     v       v
                Lock Block  Return
                     |      Error Status
                     v
               NVM_REQ_OK
```

**Architecture components:**

- **Application Layer:** Requests data to be stored in a memory block.
- **Protection Module:** Checks the block ID, write-once configuration, and lock status.
- **Memory Write Layer:** Performs the actual memory write operation.
- **Status Handler:** Reports whether the request succeeded or failed.

## 4. Hardware and Software Requirements

### Hardware Requirements

| Component | Description |
|---|---|
| Microcontroller | NXP S32K144 |
| Development Board | S32K144-based development board |
| Non-volatile memory | Internal flash or external EEPROM, depending on the implementation |
| Debug interface | Supported S32DS debugging interface |
| Computer | PC or laptop for development |

### Software Requirements

| Software | Purpose |
|---|---|
| S32 Design Studio (S32DS) | Embedded software development |
| Embedded C | Implementation of protection logic |
| S32K144 SDK / drivers | MCU peripheral and memory access, if used |
| Git and GitHub | Source-code management and project hosting |

## 5. Working Principle

1. **Initialization:** Initialize the write-once configuration and block-lock status.
2. **Request validation:** Receive a write request and verify that the block ID is within the valid range.
3. **Protection check:** Determine whether write-once protection is enabled for the requested block.
4. **Lock verification:** If the block is already locked, reject the write request.
5. **First write:** If the block is eligible for writing, perform the memory operation.
6. **Locking:** Lock the block only after the write operation succeeds.
7. **Status reporting:** Return `NVM_REQ_OK` for a successful write and `NVM_REQ_NOT_OK` for a rejected or failed request.

This ensures that a protected block cannot be modified repeatedly through the normal write-request interface.

## 6. Example Configuration

The following is an illustrative configuration for a system with four memory blocks.

| Block ID | Write-Once Enabled | Initial Lock Status |
|---|---|---|
| 0 | Yes | Unlocked |
| 1 | Yes | Unlocked |
| 2 | No | Unlocked |
| 3 | Yes | Unlocked |

Example protection logic in C:

```c
uint8_t NvM_CheckWriteProtection(uint16_t blockId)
{
    if (blockId >= total_pages)
    {
        return NVM_REQ_NOT_OK;
    }

    if ((WriteOnceEnabled[blockId] == 1U) &&
        (BlockLocked[blockId] == 1U))
    {
        return NVM_REQ_NOT_OK;
    }

    return NVM_REQ_OK;
}
```

**Example behaviour:**

- Block 0: First write is permitted; subsequent writes are rejected after it is locked.
- Block 1: Follows the same write-once policy.
- Block 2: Write-once protection is disabled, so it follows the normal configured write policy.
- Block 3: Follows the same write-once policy as Block 0.

The protection-check function only validates permission. The calling code must perform the actual write and update `BlockLocked[blockId]` after a confirmed successful write.

## 7. Build and Run

1. Open S32 Design Studio and import the S32K144 project.
2. Verify the compiler settings, device configuration, and required SDK or driver dependencies.
3. Include the write-once protection source and header files in the build.
4. Configure the block count, protection flags, and initial lock states.
5. Build the project and resolve any compilation or linking errors.
6. Connect the development board and flash the generated firmware.
7. Start debugging and issue a write request to a protected block.
8. Verify that the first successful write is accepted.
9. Issue another write request to the same block and verify that it is rejected.
10. Test invalid block IDs and unprotected blocks to confirm the expected behaviour.

## 8. Scope of the Project

- Block-level write-once protection for selected non-volatile memory blocks.
- Validation of memory block IDs before write operations.
- Prevention of repeated writes through the protected software interface.
- Integration with an embedded application running on the NXP S32K144.
- A foundation for more advanced memory protection and configuration management.

## 9. Expected Results

| Test Case | Expected Result |
|---|---|
| First write to an enabled, unlocked block | Write succeeds |
| Second write to the same locked block | Write is rejected |
| Write request with an invalid block ID | Request is rejected |
| Write request to an unprotected block | Normal write policy applies |
| Actual memory write fails | Block must remain unlocked unless the policy specifies otherwise |

The implementation is expected to prevent repeated write requests to protected blocks and provide clear status information for debugging and validation.

## 10. Limitations

- If lock status is stored only in RAM, it may be lost after a reset or power cycle.
- The protection mechanism does not automatically prevent writes performed by code that bypasses the protection module.
- The actual memory driver must correctly report write completion before the block is locked.
- Concurrent write requests require suitable synchronization to prevent multiple requests from passing the check simultaneously.
- The software mechanism alone does not provide hardware-level security against malicious or privileged code.
- Persistent write-once behaviour requires reliable storage of lock status and recovery handling across power interruptions.

## 11. Conclusion

The Write-Once Protection project demonstrates a simple and effective method for controlling repeated write access to selected non-volatile memory blocks in an embedded system. By validating block IDs, checking protection settings, and locking a block after a successful write, the mechanism helps prevent unintended modifications to stored data.

Implemented in Embedded C for the NXP S32K144 platform using S32 Design Studio, this project provides a foundation for developing more robust memory-management mechanisms. Future improvements can include persistent lock-state storage, power-failure recovery, concurrency control, and integration with a complete AUTOSAR NvM stack where required.
