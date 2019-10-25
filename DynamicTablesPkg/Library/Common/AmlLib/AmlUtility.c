/** @file
  Aml Utility.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTree.h"

#include <Library/AmlLib/AmlInterface.h>

/** This function computes and updates the ACPI table checksum.

  @param  [in, out] AcpiTable  Pointer to an Acpi table.

  @retval EFI_SUCCESS   The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AcpiPlatformChecksum (
  IN  OUT  EFI_ACPI_DESCRIPTION_HEADER  * AcpiTable
  )
{
  UINT8   * Ptr;
  UINT8     Sum;
  UINT32    Size;

  if (AcpiTable == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  Ptr = (UINT8*) AcpiTable;
  Size = AcpiTable->Length;
  Sum = 0;

  // Set the checksum field to 0 first.
  AcpiTable->Checksum = 0;

  // Compute the checksum.
  while ((Size--) != 0) {
    Sum = (Sum + (*Ptr++));
  }

  // Set the checksum.
  AcpiTable->Checksum = (0xff - Sum + 1);

  return EFI_SUCCESS;
}

/** A callback function that computes the size of a Node and adds it to the
    Size pointer stored in the Context.
    Calling this function on the root node will compute the total size of the
    AML bytestream.

  @param  [in]      Node      Node to compute the size.
  @param  [in, out] Context   Pointer holding the computed size.
                              (UINT32 *) Context.
  @param  [in, out] Status    As input, contains the Status returned by the
                              last call to this exact function during
                              the enumeration.
                              As ouput, contains the returned status of the
                              call to this function.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
BOOLEAN
EFIAPI
AmlComputeSizeCallback (
  IN      AML_NODE_HEADER  * Node,
  IN  OUT VOID             * Context,
  IN  OUT EFI_STATUS       * Status
  )
{
  UINT32  Size;

  if ((Node == NULL) || (Context == NULL)) {
    ASSERT (0);
    if (Status != NULL) {
      *Status = EFI_INVALID_PARAMETER;
    }
    return FALSE;
  }

  Size = *((UINT32*)Context);

  if (IS_AML_DATA_NODE(Node)) {
    Size += ((AML_DATA_NODE*)Node)->Size;
  } else if (IS_AML_OBJECT_NODE(Node)) {
    Size += (((AML_OBJECT_NODE*)Node)->AmlByteEncoding->OpCode ==
               AML_EXT_OP) ? 2 : 1;

    // Add the size of the PkgLen.
    if (AmlObjectNodeHasAttribute (
          (AML_OBJECT_NODE*)Node,
          AML_HAS_PKG_LENGTH)) {
      Size += AmlComputePkgLengthWidth (((AML_OBJECT_NODE*)Node)->PkgLen);
    }
  }

  *((UINT32*)Context) = Size;

  if (Status != NULL) {
    *Status = EFI_SUCCESS;
  }

  // Continue Enumeration.
  return TRUE;
}

/** Compute the size of a tree/sub-tree.

  @param  [in]      Node      Node to compute the size.
  @param  [in, out] Context   Pointer holding the computed size.
                              (UINT32 *) Context.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlComputeSize (
  IN      CONST AML_NODE_HEADER   * Node,
  IN  OUT       UINT32            * Size
  )
{
  EFI_STATUS  Status;

  if (!IS_AML_HEADER(Node)  ||
      (Size == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  AmlEnumTree (
    (AML_NODE_HEADER*)Node,
    AmlComputeSizeCallback,
    (VOID*)Size,
    &Status
    );

  return Status;
}

/** Check whether a Node is an integer node.

  By integer node we mean an object node having one of the following opcode:
    * AML_BYTE_PREFIX
    * AML_WORD_PREFIX
    * AML_DWORD_PREFIX
    * AML_QWORD_PREFIX

  @param  [in]  Node  The node to check.

  @retval TRUE  The Node is an integer node.
  @retval FALSE Otherwise.
*/
STATIC
BOOLEAN
EFIAPI
IsIntegerNode (
  IN  AML_OBJECT_NODE   * Node
)
{
  UINT8   OpCode;

  if (!IS_AML_OBJECT_NODE (Node)) {
    ASSERT (0);
    return FALSE;
  }

  // Check Node is an integer node.
  OpCode = Node->AmlByteEncoding->OpCode;
  if ((OpCode != AML_BYTE_PREFIX)   &&
      (OpCode != AML_WORD_PREFIX)   &&
      (OpCode != AML_DWORD_PREFIX)  &&
      (OpCode != AML_QWORD_PREFIX)) {
    ASSERT (0);
    return FALSE;
  }

  return TRUE;
}

/** Get the value contained in an integer node.

  @param  [in]  Node    Pointer to an integer node.
                        Must be an object node.
  @param  [out] Value   Value contained in the integer node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlNodeGetIntegerValue (
  IN  AML_OBJECT_NODE   * Node,
  OUT UINT64            * Value
  )
{
  AML_DATA_NODE  * DataNode;

  if (!IsIntegerNode (Node)   ||
      (Value == NULL)) {
    ASSERT(0);
    return EFI_INVALID_PARAMETER;
  }

  // The value is in the first fixed argument.
  DataNode = (AML_DATA_NODE*)Node->FixedArgs[AML_OP_PARSE_INDEX_GET_TERM1];

  if (!IS_AML_DATA_NODE (DataNode) ||
      (DataNode->DataType != EFI_ACPI_NODE_TYPE_UINT)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  switch (DataNode->Size) {
    case 1:
    {
      *Value = *((UINT8*)(DataNode->Buffer));
      break;
    }
    case 2:
    {
      *Value = *((UINT16*)(DataNode->Buffer));
      break;
    }
    case 4:
    {
      *Value = *((UINT32*)(DataNode->Buffer));
      break;
    }
    case 8:
    {
      *Value = *((UINT64*)(DataNode->Buffer));
      break;
    }
    default:
    {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  } // switch

  return EFI_SUCCESS;
}

/** Set the value contained in an integer node.

  The OpCode is updated accordingly to the new value
  (e.g.: AML_WORD_PREFIX for an UINT16 value).

  Checks must be done in the caller function that the new data node has a
  valid size in the AML grammar (i.e. an AML_UINT64 cannot replace an
  AML_UINT16).

  @param  [in]  Node            Pointer to an integer node.
                                Must be an object node.
  @param  [in]  NewValue        New value to write in the integer node.
  @param  [in]  HasFixedWidth   The integer has a fixed width.
                                Fixed arguments described as UINTx have a
                                fixed width.
                                Only fixed arguments described as AML_OBJECT
                                which resolve to integers have a variable
                                width.
  @param  [out] ValueWidthDiff  Difference in number of bytes used to store
                                the new value.
                                Can be negative.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
STATIC
EFI_STATUS
EFIAPI
AmlNodeSetIntegerValue (
  IN  AML_OBJECT_NODE   * Node,
  IN  UINT64              NewValue,
  IN  BOOLEAN             HasFixedWidth,
  OUT INT8              * ValueWidthDiff
  )
{
  EFI_STATUS       Status;
  AML_DATA_NODE  * DataNode;

  UINT8            NewOpCode;
  UINT8            NumberOfBytes;

  if (!IsIntegerNode (Node)   ||
      (ValueWidthDiff == NULL)) {
    ASSERT(0);
    return EFI_INVALID_PARAMETER;
  }

  // The value is in the first fixed argument.
  DataNode = (AML_DATA_NODE*)Node->FixedArgs[AML_OP_PARSE_INDEX_GET_TERM1];

  if (!IS_AML_DATA_NODE (DataNode) ||
      (DataNode->DataType != EFI_ACPI_NODE_TYPE_UINT)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Check the number of bits needed to represent the value.
  if (NewValue > MAX_UINT32) {
    // Value is 64 bits.
    NewOpCode = AML_QWORD_PREFIX;
    NumberOfBytes = 8;
  } else if (NewValue > MAX_UINT16) {
    // Value is 32 bits.
    NewOpCode = AML_DWORD_PREFIX;
    NumberOfBytes = 4;
  } else if (NewValue > MAX_UINT8) {
    // Value is 16 bits.
    NewOpCode = AML_WORD_PREFIX;
    NumberOfBytes = 2;
  } else {
    // Value is 8 bits.
    NewOpCode = AML_BYTE_PREFIX;
    NumberOfBytes = 1;
  }

  // Fixed argument have an expected format.
  // If this format is an AML_OBJECT which resolves to an integer
  // (e.g.: first fixed argument of a BufferOp), then the integer can be an
  // UINT8, UINT16, UINT32 or UINT64.
  // If the expected format has a fixed size (e.g.: first fixed argument of a
  // PackageOp), then the width of the value cannot change.
  if ((!HasFixedWidth) && (Node->AmlByteEncoding->OpCode != NewOpCode)) {
    ASSERT (0);
    return EFI_UNSUPPORTED;
  }

  *ValueWidthDiff = NumberOfBytes - DataNode->Size;
  Node->AmlByteEncoding = AmlGetByOpByte (&NewOpCode);

  // Can cast to (UINT8*) because of little endianness.
  Status = AmlUpdateDataNode (DataNode, (UINT8*)&NewValue, NumberOfBytes);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}


/** Increment/decrement the value contained in the IntegerNode.

  @param  [in]  IntegerNode     Pointer to an object node containing
                                an integer.
  @param  [in]  Operation       Choose the operation to do with the Diff:
                                * TRUE: Increment the integer.
                                * FALSE: Decrement the integer.
  @param  [in]  Diff            Value to add/substract to the integer.
  @param  [in]  HasFixedWidth   The integer has a fixed width.
                                Fixed arguments described as UINTx have a
                                fixed width.
                                Only fixed arguments described as AML_OBJECT
                                which resolve to integers have a variable
                                width.
  @param  [out] ValueWidthDiff  When modifying the integer, it can be
                                promoted/devoted, e.g. from UINT8 to UINT16.
                                Stores the change in width.
                                Can be negative.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlNodeUpdateIntegerValue (
  IN  AML_OBJECT_NODE       * IntegerNode,
  IN  BOOLEAN                 Operation,
  IN  UINT64                  Diff,
  IN  BOOLEAN                 HasFixedWidth,
  OUT INT8                  * ValueWidthDiff
  )
{
  EFI_STATUS       Status;
  UINT64           Value;

  if (ValueWidthDiff == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Get the current value.
  // Checks on the IntegerNode are done in the call.
  Status = AmlNodeGetIntegerValue (IntegerNode, &Value);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Check for UINT64 over/underflow.
  if ((Operation && (Value > (MAX_UINT64 - Diff)))  ||
      (!Operation && (Value < Diff))) {
    ASSERT(0);
    return EFI_INVALID_PARAMETER;
  }

  // Compute the new value.
  if (Operation) {
    Value += Diff;
  } else {
    Value -= Diff;
  }

  Status = AmlNodeSetIntegerValue (
             IntegerNode,
             Value,
             HasFixedWidth,
             ValueWidthDiff
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  return EFI_SUCCESS;
}

/** Propogate the size information up the tree.

  The length of the ACPI table is updated in the RootNode,
  but not the checksum.

  @param  [in]  Node        Pointer to a node.
                            Must be a root node or an object node.
  @param  [in]  Operation   Choose the operation to do with the Size:
                            * TRUE: Increment the Node's size.
                            * FALSE: Decrement the Node's size.
  @param  [in]  Diff        Value to add/substract to the Node's size.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlPropagateSize (
  IN  AML_NODE_HEADER   * Node,
  IN  BOOLEAN             Operation,
  IN  UINT32            * Diff
  )
{
  EFI_STATUS         Status;
  AML_NODE_HEADER  * ArgNode;
  AML_OBJECT_NODE  * ObjectNode;

  UINT32             Value;
  INT8               FieldWidthChange;

  if (!IS_AML_OBJECT_NODE (Node) &&
      !IS_AML_ROOT_NODE (Node)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  if (IS_AML_OBJECT_NODE (Node)) {
    ObjectNode = (AML_OBJECT_NODE*)Node;
    ArgNode = ObjectNode->FixedArgs[AML_OP_PARSE_INDEX_GET_TERM1];

    if (ObjectNode->AmlByteEncoding->OpCode == AML_BUFFER_OP) {
      // First fixed argument of BufferOp is an integer
      // (can be a BYTE, WORD, DWORD or QWORD).
      Status = AmlNodeUpdateIntegerValue (
                 (AML_OBJECT_NODE*)ArgNode,
                 Operation,
                 (UINT64)(*Diff),
                 TRUE,
                 &FieldWidthChange
                 );
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }

      // Check for UINT64 over/underflow.
      // FieldWidthChange is negative if !Operation
      if ((Operation && (*Diff > MAX_UINT64 - FieldWidthChange))  ||
         (!Operation && (*Diff < (UINT8)-FieldWidthChange))) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }

      // Increment the Size if the field width changed.
      *Diff += ABS(FieldWidthChange);
    } // AML_BUFFER_OP node.

    // Update the PgkLen.
    // Needs to be done at last to reflect the potential field width changes.
    if (AmlObjectNodeHasAttribute (ObjectNode, AML_HAS_PKG_LENGTH)) {
      Value = ObjectNode->PkgLen;

      // Check for a over/underflows.
      // PkgLen maximum value is 2^^28.
      if ((!Operation && (Value < *Diff))   ||
          (Operation && ((~(0xF << 28) - Value) < *Diff))) {
        ASSERT (0);
        return EFI_INVALID_PARAMETER;
      }

      // Update the size.
      if (Operation) {
        Value += *Diff;
      } else {
        Value -= *Diff;
      }

      ObjectNode->PkgLen = Value;
    } // PkgLen update.

    // Propagate the size up the tree.
    Status = AmlPropagateSize (
               Node->Parent,
               Operation,
               Diff
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }

  } else if (IS_AML_ROOT_NODE (Node)) {
    Value = ((AML_ROOT_NODE*)Node)->SdtHeader->Length;

    // Check for a over/underflows.
    if ((Operation && (Value > (MAX_UINT32 - *Diff)))  ||
        (!Operation && (Value < *Diff))) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    // Update the size.
    if (Operation) {
      Value += *Diff;
    } else {
      Value -= *Diff;
    }

    ((AML_ROOT_NODE*)Node)->SdtHeader->Length = Value;
  }

  return EFI_SUCCESS;
}


/** Propogate the node count information up the tree.

  @param  [in]  ObjectNode        Pointer to an object node.
  @param  [in]  Operation         Choose the operation to do
                                  with the NodeCount:
                                  * TRUE: Increment number of nodes.
                                  * FALSE: Decrement number of nodes.
  @param  [in]  NodeCount         Number of nodes added/removed (depends on the
                                  value of Operation).
  @param  [out] ValueWidthDiff  When modifying the integer, it can be
                                promoted/devoted, e.g. from UINT8 to UINT16.
                                Stores the change in width.
                                Can be negative.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlPropagateNodeCount (
  IN  AML_OBJECT_NODE   * ObjectNode,
  IN  BOOLEAN             Operation,
  IN  UINT8               NodeCount,
  OUT INT8              * FieldWidthChange
  )
{
  EFI_STATUS         Status;
  AML_NODE_HEADER  * ArgNode;
  AML_DATA_NODE    * DataNode;
  UINT32             Value;

  if (!IS_AML_OBJECT_NODE (ObjectNode)  ||
      (FieldWidthChange == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *FieldWidthChange = 0;
  ArgNode = ObjectNode->FixedArgs[AML_OP_PARSE_INDEX_GET_TERM1];
  if ((ObjectNode->AmlByteEncoding->OpCode == AML_PACKAGE_OP)) {
    // First fixed argument of PackageOp stores the number of elements
    // in the package. It is an UINT8.
    ASSERT (
      ObjectNode->AmlByteEncoding->Format[AML_OP_PARSE_INDEX_GET_TERM1] ==
      AML_UINT8
      );

    DataNode = (AML_DATA_NODE*)ArgNode;
    Value = *(DataNode->Buffer);

    // Check for over/underflow.
    if ((!Operation && (Value < NodeCount)) ||
        (Operation && ((MAX_UINT8 - Value) < NodeCount))) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    // Update the size.
    if (Operation) {
      Value += NodeCount;
    } else {
      Value -= NodeCount;
    }

    // Update the number of elements in the package.
    *(DataNode->Buffer) = Value;

  } else if (ObjectNode->AmlByteEncoding->OpCode == AML_VAR_PACKAGE_OP) {
    // First fixed argument of PackageOp stores the number of elements
    // in the package. It is an integer (can be a BYTE, WORD, DWORD, QWORD).
    Status = AmlNodeUpdateIntegerValue (
                (AML_OBJECT_NODE*)ArgNode,
                Operation,
                NodeCount,
                FALSE,
                FieldWidthChange
                );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/** Propogate information up the tree.

  The information can be a new size, a new number of arguments.

  @param  [in]  Node        Pointer to a node.
                            Must be a root node or an object node.
  @param  [in]  Operation   Choose the operation to do:
                            * TRUE:   Increment the Node's size and
                                      the Node's count
                            * FALSE:  Decrement the Node's size and
                                      the Node's count
  @param  [in]  Diff        Value to add/substract to the Node's size.
  @param  [in]  NodeCount   Number of nodes added/removed.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlPropagateInformation (
  IN  AML_NODE_HEADER   * Node,
  IN  BOOLEAN             Operation,
  IN  UINT32              Diff,
  IN  UINT8               NodeCount
  )
{
  EFI_STATUS  Status;
  INT8        FieldWidthChange;

  // Propogate the node count first as it may have an impact on the
  // number of bytes needed to store node count.
  if ((NodeCount != 0) &&
      IS_AML_OBJECT_NODE(Node)) {
    Status = AmlPropagateNodeCount (
               (AML_OBJECT_NODE*)Node,
               Operation,
               NodeCount,
               &FieldWidthChange
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }

    // Propagate the potential field width change.
    // Maximum change is between UINT8/UINT64: 7 bytes.
    if ((ABS(FieldWidthChange) > 7)             ||
        (Operation && (FieldWidthChange < 0))   ||
        (!Operation && (FieldWidthChange > 0))) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
    Diff += FieldWidthChange;
  }

  if (Diff != 0) {
    Status = AmlPropagateSize (Node, Operation, &Diff);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }
  }
  return EFI_SUCCESS;
}

