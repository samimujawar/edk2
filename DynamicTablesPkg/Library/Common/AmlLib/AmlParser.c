/** @file
  Aml Parser.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
  - Rd or RD   - Resource Data
  - Rds or RDS - Resource Data Small
  - Rdl or RDL - Resource Data Large
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlResourceDataParser.h"
#include "AmlString.h"
#include "AmlTree.h"
#include "AmlOption.h"

#include <Library/AmlLib/AmlInterface.h>
#include <Library/AmlLib/AmlPrint.h>

/*
  AML Tree
  --------

  Each ASL Statement is represented in AML as and ObjectNode
  Each ObjectNode represents an Opcode and has upto six FixedArguments
  followed by a list of VariableArguments

  A RootNode is a special type of Object Node that does not have an
  Opcode or Fixed Arguments. It only has a list of VariableArguments

  A DataNode consists of a data buffer.

  FixedArguments or VariableArguments can be either an ObjectNode or
  a DataNode.

(Node)                                            # RootNode or ObjectNode
    \
     |- [0][1][2][3][4][5]                        # Fixed Arguments
     |- {(VarArg1)->(VarArg2)->(VarArg3)->...N}   # Variable Arguments
            \-*DataNode*                          # Data Node


(Node)                                            # RootNode or ObjectNode
    \
     |- [0][1][2][3][4][5]                        # Fixed Arguments
     |             \
     |              |-[3.0][3.1][3.2][3.3][3.4][3.5]
     |              |-{(3.VarArg1)->(3.VarArg2)->(3.VarArg3)->...N}
     |
     |- {(VarArg1)->(VarArg2)->(VarArg3)->...}    # Variable Arguments
           \-*DataNode*   \
                           |-[VA2.0][VA2.1][VA2.2][VA2.3][VA2.4][VA2.5]
                           |-{(VA2.VarArg1)->(VA2.VarArg2)->(VA2.VarArg3)->...N}
                                 \-*VA2.DataNode*

*/

// Forward declaration

STATIC
EFI_STATUS
EFIAPI
AmlParseStatements (
  IN  CONST  UINT8             * Buffer,
  IN         UINT32              MaxBufferSize,
  OUT        UINT32            * Offset,
  OUT        AML_NODE_HEADER  ** OutNode
);

/** Parse a node that has a byte list.
    According to the OpCode of the Node, and the content of the buffer,
    create data nodes and add them to the variable list of arguments.

  @param  [in]  Node            Node having a byte list.
  @param  [in]  Buffer          Pointer to the beginning of the byte list.
  @param  [in]  MaxBufferSize   Maximum size allowed to parse.
  @param  [out] Offset          Pointer holding the number of bytes consumed
                                from the MaxBufferSize.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing the ResourceDataBuffer.
**/
STATIC
EFI_STATUS
EFIAPI
AmlParseByteList (
  IN         AML_OBJECT_NODE  * Node,
  IN  CONST  UINT8            * Buffer,
  IN         UINT32             MaxBufferSize,
  OUT        UINT32           * Offset
  )
{
  EFI_STATUS          Status;
  AML_NODE_HEADER   * NewNode;

  // Check if the node is an Object Node and has byte list.
  if (!AmlObjectNodeHasAttribute (Node, AML_HAS_BYTE_LIST)  ||
      (Buffer == NULL)                                      ||
      (MaxBufferSize == 0)                                  ||
      (Offset == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Check that the Node represents BufferOp
  if (Node->AmlByteEncoding->OpCode == AML_BUFFER_OP) {
    // The buffer contains a list of resource data elements.
    if (AmlRdIsResourceDataBuffer (Buffer, MaxBufferSize)) {
      // Add resource data nodes.
      Status = AmlParseResourceData (Node, Buffer, MaxBufferSize);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }
    } else {
      // The buffer doesn't contain a list of resource data elements.
      // Create a single node holding the whole buffer data.
      Status = AmlCreateDataNode (
                 EFI_ACPI_NODE_TYPE_RAW,
                 Buffer,
                 MaxBufferSize,
                 (AML_DATA_NODE**)&NewNode
                 );
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }

      Status = AmlVarListAddTailInternal (
                 (AML_NODE_HEADER*)Node,
                 NewNode
                 );
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        AmlDeleteTree (NewNode);
        return Status;
      }

      DumpRaw (Buffer, MaxBufferSize);
    }
  } else {
    // This is a field list, having on of the following OpCode:
    // * FieldOp
    // * IndexFieldOp
    // * BankFieldOp
    // The CreatexxxOp operations expect a list of fields
    // in the AML bytestream.
    //Status = ParseFieldList (Node, Buffer, MaxBufferSize);
    //if (EFI_ERROR (Status)) {
    //  ASSERT (0);
    //}

    // TODO Temporary solution:
    // TODO Create a single node holding the whole data.
    Status = AmlCreateDataNode (
               EFI_ACPI_NODE_TYPE_RAW,
               Buffer,
               MaxBufferSize,
               (AML_DATA_NODE**)&NewNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }

    Status = AmlVarListAddTailInternal (
               (AML_NODE_HEADER*)Node,
               NewNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      AmlDeleteTree (NewNode);
      return Status;
    }

    DumpRaw (Buffer, MaxBufferSize);
  }

  *Offset = MaxBufferSize;
  return EFI_SUCCESS;
}

/** Parse the NameString contained in the Buffer, and return the created node.

  @param  [in]  Buffer          Pointer to the first Byte of the NameString.
  @param  [in]  MaxBufferSize   Maximum size allowed to parse.
  @param  [out] Offset          Pointer holding the number of bytes consumed
                                from the MaxBufferSize.
  @param  [out] OutNode         Pointer holding the node created.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
STATIC
EFI_STATUS
EFIAPI
AmlParseNameString (
  IN  CONST UINT8               * Buffer,
  IN        UINT32                MaxBufferSize,
  OUT       UINT32              * Offset,
  OUT       AML_NODE_HEADER    ** OutNode
  )
{
  EFI_STATUS          Status;
  UINT32              DataSize;
  AML_NODE_HEADER   * Node;

  if ((Buffer == NULL)                                               ||
      ((AmlGetByOpByte (Buffer)->Attribute & AML_IS_NAME_CHAR) == 0) ||
      (MaxBufferSize == 0)                                           ||
      (Offset == NULL)                                               ||
      (OutNode == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Parse the NameString.
  Status = AmlGetNameStringSize (Buffer, &DataSize);
  if ((EFI_ERROR (Status)) || (DataSize > MaxBufferSize)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  Status = AmlCreateDataNode (
             AML_NAME,
             Buffer,
             DataSize,
             (AML_DATA_NODE**)&Node
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  DumpRaw (Buffer, DataSize);

  *OutNode = Node;
  *Offset = DataSize;
  return EFI_SUCCESS;
}

/** Parse the list of fixed arguments of the input Node.
    For each argument, create a node and add it to the fixed argument list
    of the Node.
    If a fixed argument has children, parse them.

  @param  [in]  Node            Node to parse the fixed arguments from.
  @param  [in]  Buffer          Pointer to the first Byte of the first fixed
                                argument.
  @param  [in]  MaxBufferSize   Maximum size allowed to parse.
  @param  [out] Offset          Pointer holding the number of bytes consumed
                                from the MaxBufferSize.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing.
**/
STATIC
EFI_STATUS
EFIAPI
AmlParseFixedArguments (
  IN  CONST AML_OBJECT_NODE   * Node,
  IN  CONST UINT8             * Buffer,
  IN        UINT32              MaxBufferSize,
  OUT       UINT32            * Offset
  )
{
  EFI_STATUS            Status;
  AML_NODE_HEADER     * ChildNode;
  AML_OP_PARSE_INDEX    TermIndex;
  AML_OP_PARSE_INDEX    MaxIndex;
  UINT32                ParsedBytes;

  AML_OP_PARSE_FORMAT   DataType;
  UINT8               * Data;
  UINT32                DataSize;

  if (!IS_AML_OBJECT_NODE (Node) ||
      (Buffer == NULL)           ||
      (MaxBufferSize == 0)       ||
      (Offset == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParsedBytes = 0;
  TermIndex = AML_OP_PARSE_INDEX_GET_TERM1;
  // Parse all the FixedArgs.
  MaxIndex = AmlGetFixedArgumentCount ((AML_OBJECT_NODE*)Node);
  while ((TermIndex < MaxIndex) &&
         ParsedBytes < MaxBufferSize) {
    Status = AmlParseOptionTerm (
               Node->AmlByteEncoding,
               Buffer + ParsedBytes,
               MaxBufferSize - ParsedBytes,
               TermIndex,
               &DataType,
               &Data,
               &DataSize
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    // No more fixed arguments.
    if (DataType == AML_NONE) {
      break;
    }

    ParsedBytes += DataSize;

    // If one of the FixedArgs is an Object node,
    // then parse the Object recursively and create a sub-tree.
    if (DataType == AML_OBJECT) {
      Status = AmlParseStatements (
                 Data,
                 DataSize,
                 &DataSize,
                 &ChildNode
                 );
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }

    } else {
    // Else the FixedArg has Data. So just add a Data Node.
      Status = AmlCreateDataNode (
                 AmlTypeToAcpiType (DataType),
                 Data,
                 DataSize,
                 (AML_DATA_NODE**)&ChildNode
                 );
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        return Status;
      }
      DumpRaw (Data, DataSize);
    }

    // Add the fixed argument ChildNode (Object or Data node).
    Status = AmlSetFixedArgument (
               (AML_OBJECT_NODE*)Node,
               TermIndex,
               ChildNode
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      AmlDeleteTree (ChildNode);
      return Status;
    }

    TermIndex++;
  } // while loop.

  // Check the loop didn't finished because of an overflow.
  if (ParsedBytes > MaxBufferSize) {
    ASSERT (0);
    return EFI_NOT_FOUND;
  }

  *Offset = ParsedBytes;
  return EFI_SUCCESS;
}

/** Parse the variable list of arguments of the input Node.
    For each argument, create a node and add it to the variable list of
    arguments of the Node.
    If a variable argument has children, parse them.

  @param  [in]  Node            Node to parse the fixed arguments from.
  @param  [in]  Buffer          Pointer to the first Byte of the first
                                variable argument.
  @param  [in]  MaxBufferSize   Maximum size allowed to parse.
  @param  [out] Offset          Pointer holding the number of bytes consumed
                                from the MaxBufferSize.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing.
**/
STATIC
EFI_STATUS
EFIAPI
AmlParseVariableArguments (
  IN        AML_OBJECT_NODE   * Node,
  IN  CONST UINT8             * Buffer,
  IN        UINT32              MaxBufferSize,
  OUT       UINT32            * Offset
  )
{
  EFI_STATUS          Status;

  AML_NODE_HEADER   * ChildNode;
  UINT32              LocalOffset;
  UINT32              ParsedBytes;

  if (!AmlObjectNodeHasAttribute (Node, AML_HAS_CHILD_OBJ) ||
      (Buffer == NULL)                                     ||
      (MaxBufferSize == 0)                                 ||
      (Offset == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  ParsedBytes = 0;
  // Parse variable arguments one by one while MaxBufferSize is not reached.
  while (ParsedBytes < MaxBufferSize) {
    Status = AmlParseStatements (
                Buffer,
                MaxBufferSize,
                &LocalOffset,
                &ChildNode
                );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      return Status;
    }

    // Add the ChildNode to the ObjectNode variable list of arguments.
    Status = AmlVarListAddTailInternal ((AML_NODE_HEADER*)Node, ChildNode);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      AmlDeleteTree (ChildNode);
      return Status;
    }

    Buffer += LocalOffset;
    ParsedBytes += LocalOffset;
  } // while loop.

  *Offset = ParsedBytes;
  return EFI_SUCCESS;
}

/** Parse the statement starting at Buffer.

  Create a node for this statement, then if available:
    * parse its fixed argument list
    * parse its variable list of arguments
    * parse its byte list

  @param  [in]  Buffer          Pointer to the first Byte of statement to parse.
  @param  [in]  MaxBufferSize   Maximum size allowed to parse.
  @param  [out] Offset          Pointer holding the number of bytes consumed
                                from the MaxBufferSize.
  @param  [out] OutNode         Pointer holding the node created.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing.
**/
STATIC
EFI_STATUS
EFIAPI
AmlParseStatements (
  IN  CONST UINT8             * Buffer,
  IN        UINT32              MaxBufferSize,
  OUT       UINT32            * Offset,
  OUT       AML_NODE_HEADER  ** OutNode
  )
{
  EFI_STATUS                  Status;

  AML_NODE_HEADER           * Node;
  CONST AML_BYTE_ENCODING   * AmlByteEncoding;
  CONST UINT8               * CurrentBuffer;

  UINT32                     ParsedBytes;
  UINT32                     RemainingBytes;
  UINT32                     ArgOffset;

  UINT32                     PkgLength;
  UINT32                     PkgOffset;


  if ((Buffer == NULL)      ||
      (MaxBufferSize == 0)  ||
      (Offset == NULL)      ||
      (OutNode == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *OutNode = NULL;
  Node = NULL;
  *Offset = 0;
  ParsedBytes = 0;

  // 0. Get the Aml Byte encoding of the statement.
  AmlByteEncoding = AmlGetByOpByte (Buffer);
  if (AmlByteEncoding == NULL) {
    ASSERT (0);
    return EFI_NOT_FOUND;
  }

  // 1. If this is a NameString (e.g. for a method invocation),
  //    create a data node and return.
  if ((AmlByteEncoding->Attribute & AML_IS_NAME_CHAR) != 0) {
    Status = AmlParseNameString (Buffer, MaxBufferSize, &ParsedBytes, &Node);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      goto error_handler;
    }
    // All done here return the node
    goto exit_handler;
  }

  CurrentBuffer = Buffer;

  // 2. Parse the OpCode (this is not a NameString).
  if (*CurrentBuffer == AML_EXT_OP) {
    ParsedBytes += 2;
  } else {
    ParsedBytes += 1;
  }

  if (ParsedBytes > MaxBufferSize) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  // Print the opcode.
  DumpRaw (CurrentBuffer, ParsedBytes);

  CurrentBuffer += ParsedBytes;

  // 3. Parse the PkgLength field, if present.
  if ((AmlByteEncoding->Attribute & AML_HAS_PKG_LENGTH) != 0) {
    PkgOffset = AmlGetPkgLength (CurrentBuffer, &PkgLength);
    if (PkgOffset == 0) {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }

    // Dump the entire package data
    DumpRaw (CurrentBuffer, PkgOffset);

    // Override MaxBufferSize if it is valid PkgLength.
    if ((ParsedBytes + PkgLength) <= MaxBufferSize) {
      MaxBufferSize = ParsedBytes + PkgLength;
      ParsedBytes += PkgOffset;
      CurrentBuffer += PkgOffset;
    } else {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  } else {
    PkgOffset = 0;
    PkgLength = 0;
  }

  // 4. Create an Object Node.
  Status = AmlCreateObjectNode (
             AmlByteEncoding,
             PkgLength,
             (AML_OBJECT_NODE**)&Node
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    goto error_handler;
  }

  // Check if we have anything left to parse
  RemainingBytes = MaxBufferSize - ParsedBytes;
  if (RemainingBytes == 0) {
    goto exit_handler;
  }

  // 5. Parse the fixed argument list and add them to the Node.
  //    If one of the FixedArgs is an Object node,
  //    then parse the Object recursively and create a sub-tree.
  //    Else just add a Data Node.
  Status = AmlParseFixedArguments (
             (AML_OBJECT_NODE*)Node,
             CurrentBuffer,
             RemainingBytes,
             &ArgOffset
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    goto error_handler;
  }

  ParsedBytes += ArgOffset;
  CurrentBuffer += ArgOffset;

  // Check if we have anything left to parse
  RemainingBytes = MaxBufferSize - ParsedBytes;
  if (RemainingBytes == 0) {
    goto exit_handler;
  }

  // 6. Parse variable list of arguments if present.
  if ((AmlByteEncoding->Attribute & AML_HAS_CHILD_OBJ) != 0) {
    Status = AmlParseVariableArguments (
               (AML_OBJECT_NODE*)Node,
               CurrentBuffer,
               RemainingBytes,
               &ArgOffset
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      goto error_handler;
    }

    ParsedBytes += ArgOffset;
    CurrentBuffer += ArgOffset;
  }

  // Check if we have anything left to parse
  RemainingBytes = MaxBufferSize - ParsedBytes;
  if (RemainingBytes == 0) {
    goto exit_handler;
  }

  // 7. Parse the byte list if present.
  if ((AmlByteEncoding->Attribute & AML_HAS_BYTE_LIST) != 0) {
    Status = AmlParseByteList (
               (AML_OBJECT_NODE*)Node,
               CurrentBuffer,
               RemainingBytes,
               &ArgOffset
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      goto error_handler;
    }

    ParsedBytes += ArgOffset;
    CurrentBuffer += ArgOffset;
  }

exit_handler:
  *Offset = ParsedBytes;
  *OutNode = Node;

  return Status;

error_handler:
  if (Node != NULL) {
    AmlDeleteTree (Node);
  }

  return Status;
}

/** Parse the definition block.

  The function parse the whole AML blob, starting with the SSDT header,
  and the parsing the AML bytestream.
  A tree structure is returned via the RootPtr.

  @param  [in]  DefinitionBlock   Pointer the defintion block.
  @param  [out] RootPtr           Pointer holding the root node of the tree.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
  @retval EFI_NOT_FOUND           Error while parsing.
**/
EFI_STATUS
EFIAPI
AmlParseDefinitionBlock (
  IN  CONST EFI_ACPI_DESCRIPTION_HEADER   * DefinitionBlock,
  OUT       AML_ROOT_NODE                ** RootPtr
  )
{
  UINT8             * Buffer;
  UINT8             * CurrentBuffer;
  UINT32              BufferSize;
  UINT32              BytesToParse;
  UINT32              Offset;
  EFI_STATUS          Status;
  AML_ROOT_NODE     * Root;
  AML_NODE_HEADER   * Node;

  Buffer = (VOID *)((UINTN)DefinitionBlock +
             sizeof (EFI_ACPI_DESCRIPTION_HEADER));
  BufferSize = DefinitionBlock->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER);

  // Create a root node
  Status = AmlCreateRootNode (
             (EFI_ACPI_DESCRIPTION_HEADER*)DefinitionBlock,
             &Root
             );
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  CurrentBuffer = Buffer;
  BytesToParse = BufferSize;
  Offset = 0;

  // Parse the AML bytestream.
  // All the statements parsed from here are under the root.
  while (BytesToParse > 0) {
    Status = AmlParseStatements (
               CurrentBuffer,
               BytesToParse,
               &Offset,
               &Node
               );
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      goto error_handler;
    }

    Status = AmlVarListAddTailInternal ((AML_NODE_HEADER*)Root, Node);
    if (EFI_ERROR (Status)) {
      ASSERT (0);
      AmlDeleteTree (Node);
      goto error_handler;
    }

    CurrentBuffer += Offset;
    BytesToParse -= Offset;
  } // Main Parser Loop

  *RootPtr = Root;
  return Status;

error_handler:
  if (Root != NULL) {
    AmlDeleteTree ((AML_NODE_HEADER*)Root);
  }

  return Status;
}
