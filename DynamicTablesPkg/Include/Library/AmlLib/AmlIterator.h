/** @file
  Aml Iterator.

  Copyright (c) 2019, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef AML_ITERATOR_H_
#define AML_ITERATOR_H_

/** Iterator mode.

  Modes to choose how the iterator is progressing in the tree.
  A
  \-B    <- Iterator initialized with this node.
  | \-C
  | | \-D
  | \-E
  |   \-F
  |   \-G
  \-H
    \-I
*/
typedef enum EAmlIteratorMode {
  EAmlIteratorUnknown,        ///< Unknown/Invalid Aml IteratorMode
  EAmlIteratorLinear,         ///< Iterate following the AML bytestream order.
                              ///  The order followed by the iterator would be:
                              ///  B, C, D, E, F, G, H, I, NULL
  EAmlIteratorBranch,         ///< Iterate through the node of a branch.
                              ///  The itertion is following the AML bytestream
                              ///  order.
                              ///  The order followed by the iterator would be:
                              ///  B, C, D, E, F, G, NULL
  EAmlIteratorModeMax         ///< Max Enum
} EAML_ITERATOR_MODE;

/** Iterator.

  Allows to traverse the tree in different orders.
*/
typedef struct AmlTreeIterator AML_TREE_ITERATOR;

/** Function pointer to a get the current node of the iterator.
*/
typedef
EFI_STATUS
(EFIAPI * EDKII_AML_TREE_ITERATOR_GET_NODE) (
  IN  OUT  AML_TREE_ITERATOR  * Iterator,
  OUT      AML_NODE_HANDLE    * OutNode
  );

/** Function pointer to a update the current node of the iterator
    with the next node.
*/
typedef
EFI_STATUS
(EFIAPI * EDKII_AML_TREE_ITERATOR_GET_NEXT) (
  IN  OUT  AML_TREE_ITERATOR  * Iterator
  );

/** Function pointer to a update the current node of the iterator
    with the previous node.
*/
typedef
EFI_STATUS
(EFIAPI * EDKII_AML_TREE_ITERATOR_GET_PREVIOUS) (
  IN  OUT  AML_TREE_ITERATOR  * Iterator
  );

/**  Iterator structure to traverse the tree.
*/
typedef struct AmlTreeIterator {
  /// Get the current node of the iterator.
  EDKII_AML_TREE_ITERATOR_GET_NODE      GetNode;

  /// Update the current node of the iterator with the next node.
  EDKII_AML_TREE_ITERATOR_GET_NEXT      GetNext;

  /// Update the current node of the iterator with the previous node.
  EDKII_AML_TREE_ITERATOR_GET_PREVIOUS  GetPrevious;
} AML_TREE_ITERATOR;


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
  IN   AML_NODE_HANDLE        Node,
  IN   EAML_ITERATOR_MODE     IteratorMode,
  OUT  AML_TREE_ITERATOR   ** IteratorPtr
  );

/** Delete an iterator.

  @param  [in]  Iterator  Pointer to an iterator.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   Invalid parameter.
**/
EFI_STATUS
EFIAPI
AmlDeleteIterator (
  IN  AML_TREE_ITERATOR   * Iterator
  );

#endif // AML_ITERATOR_H_
