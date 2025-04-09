/*
 * Binární vyhledávací strom — rekurzivní varianta
 *
 * S využitím datových typů ze souboru btree.h a připravených koster funkcí
 * implementujte binární vyhledávací strom pomocí rekurze.
 */

#include "../btree.h"
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
 * Funkci implementujte rekurzivně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  if (tree == NULL) // Tree is empty
    return false;

  int c_key = tree->key;

  if (c_key == key) // Node was found
  {
    *value = &tree->content;
    return true;
  }

  if (c_key > key)
    return bst_search(tree->left, key, value); // Recursion to the left subtree

  if (c_key < key)
    return bst_search(tree->right, key, value); // Recursion to the right subtree

  return false;
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
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
  if (*tree == NULL) // Tree is empty, allocating memory for new node
  {
    bst_node_t *newNode = (bst_node_t *)malloc(sizeof(bst_node_t));
    if (newNode == NULL)
      return;

    newNode->key = key;
    newNode->content = value;
    newNode->left = newNode->right = NULL;
    *tree = newNode; // Inserting root node
    return;
  }

  int c_key = (*tree)->key;

  if (c_key == key) // Replacing value in already existing node
  {
    (*tree)->content = value;
    return;
  }

  if (key < c_key) // Recursion to the left and right subtree
    bst_insert(&(*tree)->left, key, value);
  else if (key > c_key)
    bst_insert(&(*tree)->right, key, value);
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazeny klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využitá při implementaci funkce bst_delete.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  if (*tree == NULL) // Tree is empty
    return;

  if ((*tree)->right == NULL) // Found the rightmost child
  {
    target->key = (*tree)->key; // Replacing values
    target->content = (*tree)->content;

    bst_node_t *temp = *tree; // Pointing to the left child of the founded node
    *tree = (*tree)->left;

    free(temp);
    return;
  }

  bst_replace_by_rightmost(target, &(*tree)->right); // Recursion
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
 * Funkci implementujte rekurzivně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
  if (*tree == NULL) // Tree is empty
    return;

  int c_key = (*tree)->key;
  bool has_left_subtree = (*tree)->left != NULL;
  bool has_right_subtree = (*tree)->right != NULL;

  if (c_key == key) // Founded node to delete
  {

    if (!has_left_subtree && !has_right_subtree) // Node has no children
    {
      free(*tree);
      *tree = NULL;
      return;
    }

    if (has_left_subtree ^ has_right_subtree) // Node has only 1 subtree
    {
      bst_node_t *temp = *tree;
      if (has_left_subtree)
        *tree = (*tree)->left;
      else
        *tree = (*tree)->right;
      free(temp);
      return;
    }

    if (has_left_subtree && has_right_subtree) // Node has both subtrees
    {
      bst_replace_by_rightmost(*tree, &(*tree)->left);
      return;
    }
  }

  if ((key < c_key) && has_left_subtree) // Deleting left subtree
  {
    bst_delete(&(*tree)->left, key);
    return;
  }

  if ((key > c_key) && has_right_subtree) // Deleting right subtree
  {
    bst_delete(&(*tree)->right, key);
    return;
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  if (*tree == NULL) // Tree is empty
    return;

  bst_dispose(&(*tree)->left); // Deleting left and right subtree
  bst_dispose(&(*tree)->right);

  free(*tree);
  *tree = NULL;
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL)
    return;

  bst_add_node_to_items(tree, items);
  bst_preorder(tree->left, items);
  bst_preorder(tree->right, items);
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL)
    return;

  bst_inorder(tree->left, items);
  bst_add_node_to_items(tree, items);
  bst_inorder(tree->right, items);
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte rekurzivně bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
  if (tree == NULL)
    return;

  bst_postorder(tree->left, items);
  bst_postorder(tree->right, items);
  bst_add_node_to_items(tree, items);
}
