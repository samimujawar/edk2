/** @file
  Aml Tree Iterator.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Aml.h"
#include "AmlNode.h"
#include "AmlTreeTraversal.h"

#include <Library/AmlLib/AmlIterator.h>

/** Iterator to traverse the tree.

  This is an internal structure.
*/
typedef struct AmlTreeInternalIterator {
  /// External iterator structure, containing the external APIs.
  /// Must be the first field.
  AML_TREE_ITERATOR         Iterator;

  /// Pointer to the node on which the iterator has been initialized.
  CONST  AML_NODE_HEADER  * InitialNode;

  /// Pointer to the current node.
  CONST  AML_NODE_HEADER  * CurrentNode;

  /// Iteration mode.
  /// Allow to choose how to traverse the tree/choose which node is next.
  EAML_ITERATOR_MODE        Mode;
} AML_TREE_ITERATOR_INTERNAL;

/** Function checking whether the internal iterator is valid.

  @param  [in]  Iterator  Pointer to an AML_TREE_ITERATOR_INTERNAL.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlCheckIterator (
  IN  CONST  AML_TREE_ITERATOR_INTERNAL  * InternalIterator
  )
{
  // CurrentNode can be NULL, but InitialNode cannot.
  if ((InternalIterator == NULL)                              ||
      (InternalIterator->Mode <= EAmlIteratorUnknown)         ||
      (InternalIterator->Mode >= EAmlIteratorModeMax)         ||
      !IS_AML_HEADER (InternalIterator->InitialNode)          ||
      ((InternalIterator->CurrentNode != NULL)                &&
        !IS_AML_HEADER (InternalIterator->CurrentNode))) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

/** Get the current node of an iterator.

  @param  [in]  Iterator  Pointer to an iterator.
  @param  [out] OutNode   Pointer holding the next node.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlIteratorGetNode (
  IN  AML_TREE_ITERATOR   * Iterator,
  OUT AML_NODE_HEADER    ** OutNode
  )
{
  EFI_STATUS                    Status;
  AML_TREE_ITERATOR_INTERNAL  * InternalIterator;

  if (OutNode == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  InternalIterator = (AML_TREE_ITERATOR_INTERNAL*)Iterator;

  Status = AmlCheckIterator (InternalIterator);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  *OutNode = (AML_NODE_HEADER*)InternalIterator->CurrentNode;

  return EFI_SUCCESS;
}

/** Move the current node of the iterator to the next node,
    according to the iteration mode selected.

  @param  [in, out] Iterator  Pointer to an iterator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlIteratorGetNext (
  IN  OUT  AML_TREE_ITERATOR  * Iterator
  )
{
  EFI_STATUS                    Status;
  AML_TREE_ITERATOR_INTERNAL  * InternalIterator;
  AML_NODE_HEADER             * NextNode;

  InternalIterator = (AML_TREE_ITERATOR_INTERNAL*)Iterator;

  Status = AmlCheckIterator (InternalIterator);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the next node according to the iteration mode.
  switch (InternalIterator->Mode) {
    case EAmlIteratorLinear:
    {
      NextNode = AmlGetNextNode (InternalIterator->CurrentNode);
      break;
    }
    case EAmlIteratorBranch:
    {
      NextNode = AmlGetNextNode (InternalIterator->CurrentNode);
      // Check whether NextNode is a sibling of InitialNode.
      if ((NextNode != NULL)              &&
          !IS_AML_ROOT_NODE (NextNode)    &&
          (NextNode->Parent == InternalIterator->InitialNode->Parent)) {
        NextNode = NULL;
      }
      break;
    }
    default:
    {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  } // switch

  InternalIterator->CurrentNode = NextNode;

  return EFI_SUCCESS;
}

/** Move the current node of the iterator to the previous node,
    according to the iteration mode selected.

  @param  [in, out]  Iterator   Pointer to an iterator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
STATIC
EFI_STATUS
EFIAPI
AmlIteratorGetPrevious (
  IN  OUT  AML_TREE_ITERATOR  * Iterator
  )
{
  EFI_STATUS                    Status;
  AML_TREE_ITERATOR_INTERNAL  * InternalIterator;
  AML_NODE_HEADER             * PreviousNode;

  InternalIterator = (AML_TREE_ITERATOR_INTERNAL*)Iterator;

  Status = AmlCheckIterator (InternalIterator);
  if (EFI_ERROR (Status)) {
    ASSERT (0);
    return Status;
  }

  // Get the previous node according to the iteration mode.
  switch (InternalIterator->Mode) {
    case EAmlIteratorLinear:
    {
      PreviousNode = AmlGetPreviousNode (InternalIterator->CurrentNode);
      break;
    }
    case EAmlIteratorBranch:
    {
      PreviousNode = AmlGetPreviousNode (InternalIterator->CurrentNode);
      // Check wheteher PreviousNode is a sibling of InitialNode.
      if (!IS_AML_ROOT_NODE (PreviousNode) &&
        (PreviousNode->Parent == InternalIterator->InitialNode)) {
        PreviousNode = NULL;
      }
      break;
    }
    default:
    {
      ASSERT (0);
      return EFI_INVALID_PARAMETER;
    }
  } // switch

  InternalIterator->CurrentNode = PreviousNode;

  return EFI_SUCCESS;
}

/** Initialize an iterator.

  @param  [in]  Node          Pointer to the node.
  @param  [in]  IteratorMode  Selected mode to traverse the tree.
  @param  [out] IteratorPtr   Pointer holding the created iterator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
  @retval EFI_OUT_OF_RESOURCES    Could not allocate memory.
**/
EFI_STATUS
EFIAPI
AmlInitializeIterator (
  IN   AML_NODE_HEADER      * Node,
  IN   EAML_ITERATOR_MODE     IteratorMode,
  OUT  AML_TREE_ITERATOR   ** IteratorPtr
  )
{
  AML_TREE_ITERATOR_INTERNAL * InternalIterator;

  if ((Node == NULL) ||
      (IteratorMode <= EAmlIteratorUnknown) ||
      (IteratorMode >= EAmlIteratorModeMax) ||
      (IteratorPtr == NULL)) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  *IteratorPtr = NULL;
  InternalIterator = AllocatePool (sizeof(AML_TREE_ITERATOR_INTERNAL));
  if (InternalIterator == NULL) {
    ASSERT (0);
    return EFI_OUT_OF_RESOURCES;
  }

  InternalIterator->InitialNode = Node;
  InternalIterator->CurrentNode = Node;
  InternalIterator->Mode = IteratorMode;
  InternalIterator->Iterator.GetNode = AmlIteratorGetNode;
  InternalIterator->Iterator.GetNext = AmlIteratorGetNext;
  InternalIterator->Iterator.GetPrevious = AmlIteratorGetPrevious;

  *IteratorPtr = &InternalIterator->Iterator;

  return EFI_SUCCESS;
}

/** Delete an iterator.

  @param  [in]  Iterator  Pointer to an iterator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteIterator (
  IN  AML_TREE_ITERATOR   * Iterator
  )
{
  if (Iterator == NULL) {
    ASSERT (0);
    return EFI_INVALID_PARAMETER;
  }

  FreePool (Iterator);

  return EFI_SUCCESS;
}
