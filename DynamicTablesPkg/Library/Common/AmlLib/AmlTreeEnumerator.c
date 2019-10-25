/** @file
  Aml Tree Enumerator.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTree.h"

#include <Library/AmlLib/AmlInterface.h>

/** Enumerate all nodes of the subtree under the input Node in the AML
    bytestream order, and call the CallBack function with the input Context.
    The prototype of the Callback function is EDKII_AML_TREE_ENUM_CALLBACK.

  @param  [in]      Node      Enumerate nodes of the subtree under this Node.
  @param  [in]      CallBack  Callback function to call on each node.
  @param  [in, out] Context   Void pointer used to pass some information
                              to the Callback function.
  @param  [out]     Status    Optional parameter that can be used to get
                              the status of the Callback function.

  @return EFI_SUCCESS if the function completed successfully.
          Otherwise, the returned value depends on the Callback.
**/
BOOLEAN
EFIAPI
AmlEnumTree (
  IN      AML_NODE_HEADER               * Node,
  IN      EDKII_AML_TREE_ENUM_CALLBACK    CallBack,
  IN  OUT VOID                          * Context,
      OUT EFI_STATUS                    * Status  OPTIONAL
  )
{
  BOOLEAN               ContinueEnum;

  AML_OP_PARSE_INDEX    Index;
  AML_OP_PARSE_INDEX    MaxIndex;

  LIST_ENTRY          * StartLink;
  LIST_ENTRY          * CurrentLink;

  if (CallBack == NULL) {
    ASSERT (0);
    if (Status != NULL) {
      *Status = EFI_INVALID_PARAMETER;
    }
    return FALSE;
  }

  ContinueEnum = (*CallBack)(Node, Context, Status);
  if (ContinueEnum == FALSE) {
    return ContinueEnum;
  }

  // Iterate through the fixed list of arguments.
  MaxIndex = AmlGetFixedArgumentCount ((AML_OBJECT_NODE*)Node);
  for (Index = AML_OP_PARSE_INDEX_GET_TERM1; Index < MaxIndex; Index++) {
    ContinueEnum = AmlEnumTree (
                     AmlGetFixedArgument ((AML_OBJECT_NODE*)Node, Index),
                     CallBack,
                     Context,
                     Status
                     );
    if (ContinueEnum == FALSE) {
      return ContinueEnum;
    }
  }

  // Iterate through the variable list of arguments.
  StartLink = AmlNodeGetVariableArgList (Node);
  if (StartLink != NULL) {
    CurrentLink = StartLink->ForwardLink;
    while (CurrentLink != StartLink) {
      ContinueEnum = AmlEnumTree (
                       (AML_NODE_HEADER*)CurrentLink,
                       CallBack,
                       Context,
                       Status
                       );
      if (ContinueEnum == FALSE) {
        return ContinueEnum;
      }
      CurrentLink = CurrentLink->ForwardLink;
    } // while
  }

  return ContinueEnum;
}
