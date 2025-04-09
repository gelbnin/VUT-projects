/*
 * Tabulka s rozptýlenými položkami
 *
 * S využitím datových typů ze souboru hashtable.h a připravených koster
 * funkcí implementujte tabulku s rozptýlenými položkami s explicitně
 * zretězenými synonymy.
 *
 * Při implementaci uvažujte velikost tabulky HT_SIZE.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptylovací funkce která přidělí zadanému klíči index z intervalu
 * <0,HT_SIZE-1>. Ideální rozptylovací funkce by měla rozprostírat klíče
 * rovnoměrně po všech indexech. Zamyslete sa nad kvalitou zvolené funkce.
 */
int get_hash(char *key)
{
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++)
  {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializace tabulky — zavolá sa před prvním použitím tabulky.
 */
void ht_init(ht_table_t *table)
{
  for (int i = 0; i < HT_SIZE; i++)
    (*table)[i] = NULL;
}

/*
 * Vyhledání prvku v tabulce.
 *
 * V případě úspěchu vrací ukazatel na nalezený prvek; v opačném případě vrací
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key)
{
  ht_item_t *c_item = (*table)[get_hash(key)];

  // Iterating through the table
  while (c_item != NULL)
  {
    if (strcmp(c_item->key, key) == 0) // Item is found
      return c_item;
    c_item = c_item->next; // Move to the next item
  }
  return NULL;
}

/*
 * Vložení nového prvku do tabulky.
 *
 * Pokud prvek s daným klíčem už v tabulce existuje, nahraďte jeho hodnotu.
 *
 * Při implementaci využijte funkci ht_search. Pri vkládání prvku do seznamu
 * synonym zvolte nejefektivnější možnost a vložte prvek na začátek seznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value)
{
  ht_item_t *c_item = ht_search(table, key);

  if (c_item != NULL) // If item was found, then update the value
  {
    c_item->value = value;
    return;
  }
  else // If not, allocate and insert a new item
  {
    ht_item_t *newItem = (ht_item_t *)malloc(sizeof(ht_item_t));
    if (newItem == NULL)
      return;

    newItem->key = key;
    newItem->value = value;
    newItem->next = (*table)[get_hash(key)];
    (*table)[get_hash(key)] = newItem;
  }
}

/*
 * Získání hodnoty z tabulky.
 *
 * V případě úspěchu vrací funkce ukazatel na hodnotu prvku, v opačném
 * případě hodnotu NULL.
 *
 * Při implementaci využijte funkci ht_search.
 */
float *ht_get(ht_table_t *table, char *key)
{
  ht_item_t *item = ht_search(table, key);

  if (item != NULL) // Item was found
    return &(item->value);
  else
    return NULL; // Item was not found
}

/*
 * Smazání prvku z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje přiřazené k danému prvku.
 * Pokud prvek neexistuje, funkce nedělá nic.
 *
 * Při implementaci NEPOUŽÍVEJTE funkci ht_search.
 */
void ht_delete(ht_table_t *table, char *key)
{
  int hash = get_hash(key);
  ht_item_t *c_item = (*table)[hash];
  ht_item_t *r_item = NULL;

  bool found = false;
  while (c_item != NULL && !found) // Iterate until the item is found or the entire table is traversed
  {
    if (strcmp(c_item->key, key) == 0) // Item was found
    {
      if (r_item == NULL) // Deleting 1. item
        (*table)[hash] = c_item->next;
      else // Deleting item somewhere in the middle
        r_item->next = c_item->next;

      free(c_item);
      return;
    }

    r_item = c_item; // Next item
    c_item = c_item->next;
  }
}

/*
 * Smazání všech prvků z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje a uvede tabulku do stavu po
 * inicializaci.
 */
void ht_delete_all(ht_table_t *table)
{
  for (int i = 0; i < HT_SIZE; i++) // Iteration through table
  {
    ht_item_t *c_item = (*table)[i];

    if (c_item == NULL) // Item does not exitsts
      continue;

    ht_item_t *nextItem;
    while (c_item != NULL) // Iteration and deleting through elements in one hash
    {
      nextItem = c_item->next;
      free(c_item);
      c_item = nextItem;
    }

    (*table)[i] = NULL; // Deleting hash
  }
}
