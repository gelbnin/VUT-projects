/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h
 * a připravených koster funkcí implementujte binární vyhledávací
 * strom bez použití rekurze.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  if (tree == NULL) // Tree is empty
    return false;

  bst_node_t *c_node = tree;
  bool found = false;

  while (!found) // Iteration through tree
  {
    int c_key = c_node->key;
    bool has_left_subtree = c_node->left != NULL;
    bool has_right_subtree = c_node->right != NULL;

    if (key == c_key) // Node was founded
    {
      *value = &c_node->content;
      found = true;
    }
    else if (key < c_key && has_left_subtree)
      c_node = c_node->left;
    else if (key > c_key && has_right_subtree)
      c_node = c_node->right;
    else
      break; // Node was not found
  }

  return found;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
  bst_node_t *newNode = (bst_node_t *)malloc(sizeof(bst_node_t)); // Allocation
  if (newNode == NULL)
    return;

  // Setting values for the new node
  newNode->key = key;
  newNode->content = value;
  newNode->left = newNode->right = NULL;

  if (*tree == NULL) // Tree is empty
  {
    *tree = newNode;
    return;
  }

  bst_node_t *c_node = *tree;
  bool done = false;

  while (!done) // Iteration through the tree
  {
    int c_key = c_node->key;
    bool has_left_subtree = c_node->left != NULL;
    bool has_right_subtree = c_node->right != NULL;

    if (key == c_key) // Node was found
    {
      c_node->content = value; // Replacing value
      free(newNode);
      done = true;
    }

    if (key < c_key) // Left subtree
    {
      if (has_left_subtree)
        c_node = c_node->left;
      else
      { // Founded the leftmost node
        c_node->left = newNode;
        done = true;
      }
    }

    if (key > c_key) // Right subtree
    {
      if (has_right_subtree)
      {
        c_node = c_node->right;
      }
      else
      { // Found the rightmost node
        c_node->right = newNode;
        done = true;
      }
    }
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  if (*tree == NULL) // Tree is empty
    return;

  bst_node_t *c_node = *tree;
  bst_node_t *r_node = NULL;
  bool done = false;

  while (!done) // Iteration through the tree
  {
    if (c_node->right == NULL) // Founded the rightmost node
    {
      target->key = c_node->key; // Replacing values
      target->content = c_node->content;

      if (r_node == NULL) // Replacing root
        *tree = c_node->left;
      else
        r_node->right = c_node->left;

      free(c_node);
      done = true;
    }
    else // Left subtree
    {
      r_node = c_node;
      c_node = c_node->right;
    }
  }
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
  if (*tree == NULL) // Tree is empty
    return;

  bst_node_t *c_node = *tree;
  bst_node_t *r_node = NULL;
  bool done = false;

  while (!done) // Iteration through the tree
  {
    if (c_node == NULL)
      return;

    int c_key = c_node->key;
    bool has_left_subtree = c_node->left != NULL;
    bool has_right_subtree = c_node->right != NULL;

    if (key == c_key) // node was found
    {
      bool is_left_child = r_node != NULL && r_node->left->key == c_node->key;
      bool is_right_child = r_node != NULL && r_node->right->key == c_node->key;

      if (!has_left_subtree && !has_right_subtree)
      {
        free(c_node);

        if (is_left_child) // Deleting children if exists
          r_node->left = NULL;
        else if (is_right_child)
          r_node->right = NULL;
        else
          *tree = NULL;
      }

      if (has_left_subtree ^ has_right_subtree)
      {
        bst_node_t *replace_node = has_left_subtree ? c_node->left : c_node->right;
        free(c_node);

        if (is_left_child) // Deleting children if exists
          r_node->left = replace_node;
        else if (is_right_child)
          r_node->right = replace_node;
        else
          *tree = replace_node;
      }

      if (has_left_subtree && has_right_subtree)
        bst_replace_by_rightmost(c_node, &(c_node->left));
      done = true;
    }

    if (key < c_key) // Left subtree
    {
      r_node = c_node;
      c_node = c_node->left;
    }

    if (key > c_key) // Right subtree
    {
      r_node = c_node;
      c_node = c_node->right;
    }
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  if (*tree == NULL) // Tree is empty
    return;

  stack_bst_t *stack = (stack_bst_t *)malloc(sizeof(stack_bst_t)); // Stack init
  if (stack == NULL)
    return;

  stack_bst_init(stack);

  stack_bst_push(stack, *tree); // Push root to the stack

  while (!stack_bst_empty(stack)) // Iteration throught tree and freeing nodes
  {
    bst_node_t *c_node = stack_bst_pop(stack);
    if (c_node == NULL)
      continue;

    if (c_node->left != NULL) // Push left child to stack
      stack_bst_push(stack, c_node->left);

    if (c_node->right != NULL) // Push left child to stack
      stack_bst_push(stack, c_node->right);

    free(c_node);
  }

  *tree = NULL;
  free(stack);
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit, bst_items_t *items)
{
  bst_node_t *c_node = tree;

  while (c_node != NULL)
  {
    bst_add_node_to_items(c_node, items);

    stack_bst_push(to_visit, c_node);

    c_node = c_node->left;
  }
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL) // Tree is empty
    return;

  stack_bst_t *stack = (stack_bst_t *)malloc(sizeof(stack_bst_t)); // Stack init
  if (stack == NULL)
    return;
  stack_bst_init(stack);

  bst_node_t *c_node = tree;
  bst_leftmost_preorder(c_node, stack, items);

  while (!stack_bst_empty(stack))
  {
    c_node = stack_bst_pop(stack);

    if (c_node->right != NULL)
      bst_leftmost_preorder(c_node->right, stack, items);
  }

  free(stack);
}

/*
 * Pomocná funkce pro iterativní inorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit)
{
  bst_node_t *c_node = tree;

  while (c_node != NULL)
  {
    stack_bst_push(to_visit, c_node);
    c_node = c_node->left;
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL) // Tree is empty
    return;

  stack_bst_t *stack = (stack_bst_t *)malloc(sizeof(stack_bst_t)); // Stack init
  if (stack == NULL)
    return;
  stack_bst_init(stack);

  bst_node_t *c_node = tree;
  bst_leftmost_inorder(c_node, stack);

  while (!stack_bst_empty(stack))
  {
    c_node = stack_bst_pop(stack);
    bst_add_node_to_items(c_node, items);

    if (c_node->right != NULL)
      bst_leftmost_inorder(c_node->right, stack);
  }

  free(stack);
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit)
{
  bst_node_t *c_node = tree;

  while (c_node != NULL)
  {
    stack_bst_push(to_visit, c_node);
    stack_bool_push(first_visit, true);
    c_node = c_node->left;
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL) // Tree is empty
    return;

  stack_bst_t *stack = (stack_bst_t *)malloc(sizeof(stack_bst_t)); // Stack init
  if (stack == NULL)
    return;

  stack_bst_init(stack);

  stack_bool_t *bool_stack = (stack_bool_t *)malloc(sizeof(stack_bool_t));
  if (bool_stack == NULL)
    return;

  stack_bool_init(bool_stack);

  bst_node_t *c_node = tree;
  bst_leftmost_postorder(c_node, stack, bool_stack);

  while (!stack_bst_empty(stack))
  {
    c_node = stack_bst_pop(stack);
    bool is_first_visit = stack_bool_pop(bool_stack);

    if (is_first_visit)
    {
      stack_bst_push(stack, c_node);
      stack_bool_push(bool_stack, false);

      if (c_node->right != NULL)
        bst_leftmost_postorder(c_node->right, stack, bool_stack);
    }
    else
      bst_add_node_to_items(c_node, items);
  }

  free(stack);
  free(bool_stack);
}
