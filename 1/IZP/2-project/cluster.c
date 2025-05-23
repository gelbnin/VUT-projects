/**
 * Kostra programu pro 2. projekt IZP 2022/23
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

/*
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
int init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    // TODO
	c->obj = malloc(cap * sizeof(struct obj_t));
	if(c->obj == NULL)
	{
		fprintf(stderr, "SAS");
		return -1;
	}
	c->size = cap;
	c->capacity = cap;
	return 0;

}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    // TODO
	free(c->obj);
	c->size = 0;
 	c->capacity = 0;
	c->obj = NULL;
	return;
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    // TODO
	if(c->capacity == c->size)
		resize_cluster(c, c->capacity + 1);
	c->obj[c->size] = obj;
	c->size = c->size + 1;
	return;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    // TODO
	if(c1->capacity <= c1->size + c2->size)
		resize_cluster(c1, c1->size + c2->size);
	for(int i = 0; i < c2->size; i++)
	{
		append_cluster(c1, c2->obj[i]);
	}

	sort_cluster(c1);
	return;
}


/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    // TODO
	clear_cluster(&carr[idx]);
	for(int i = idx; i < narr; i++)
		carr[i] = carr[i + 1];
	return narr - 1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    // TODO
	float dx = o1->x - o2->x; 
	float dy = o1->y - o2->y; 
	return sqrt(dx*dx + dy*dy);
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    // TODO
	float min = sqrtf(1000*1000 + 1000*1000);
	for( int i = 0; i < c1->size; i++)
	{
		for(int j = 0; j < c2->size; j++)
		{
			if(obj_distance(&(c1->obj[i]), &(c2->obj[j])) < min)
				min = obj_distance(&(c1->obj[i]), &(c2->obj[j]));
		}
	}
	return min;
}

/*	
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);
	float min = sqrtf(1000*1000 + 1000*1000);
//	float min = 1000;//cluster_distance(&(carr[0]), &(carr[1]));
	for(int i = 0; i < narr; i++)
	{
		for( int j = 0; j < narr; j++)
		{
			if(i != j)
			{
				if(cluster_distance(&(carr[i]), &(carr[j])) < min)
				{
					min = cluster_distance(&(carr[i]), &(carr[j]));
					*c1 = i;
					*c2 = j;
				}
			}
		}
	}
	return;
					
    // TODO
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    // TODO
	int count;
	FILE* fp;
	fp = fopen(filename, "r");
	if(fp == NULL)
		return -1;
	if(fscanf(fp, "count=%i", &count) != 1)
		return -1;
//standartni kontrola otevreni soubouru a nacteni polozky count

	*arr = malloc(count * sizeof(struct cluster_t));
	for(int i = 0; i < count; i++)
	{
		init_cluster(&(*arr)[i], 1);
		fscanf(fp, "%i %e %e\n", &(*arr)[i].obj[0].id, &(*arr)[i].obj[0].x , &(*arr)[i].obj[0].y);
	}
	fclose(fp);
	
//kontrola stejneho id	
	int id_cnt = 0;
	for(int i = 0; i < count; i++)
	{
		for(int j = 0; j < count; j++)
		{
			if((*arr)[i].obj[0].id == (*arr)[j].obj[0].id)
				id_cnt++;
		}
	}
	if(id_cnt != count)
		return -1;
	return count;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int main(int argc, char *argv[])
{
    struct cluster_t *clusters = NULL;
    	int narr = 0;
	int arg = 0;
//kontrola celociselneho argumentu
	if(argc == 1)
	{
		fprintf(stderr, "To few arguments\n");
		return 1;
	}
	else if(argc > 3)
	{
		fprintf(stderr, "To many arguments\n");
		return 2;
	}	
	else if(argc == 2)
		arg = 1;
	else
	{
		arg = atoi(argv[2]);
		if(arg != atof(argv[2]))
		{
			fprintf(stderr, "Use only intigets in an argument 2\n");
			return 3;
		}
	}
//kontrola otevreni souboru
	narr = load_clusters(argv[1], &clusters);
	if(narr == -1)
	{
		fprintf(stderr, "FILE OPEN FAIL\n");
		return 4;
	}
//kontrola vhodnosti argumentu s poctem objektu v souboru
	if(arg == 0)
	{
		for(int i = 0; i < narr; i++)
		{
			clear_cluster(&clusters[i]);
		}
		free(clusters);
		fprintf(stderr, "Use only intigers in an argument 2\n");
		return 3;
	}
	else if(arg > narr)
	{
		for(int i = 0; i < narr; i++)
		{
			clear_cluster(&clusters[i]);
		}
		free(clusters);
		fprintf(stderr, "Argumet 2 is higher than number of objects in objekty.txt\n");
		return 5;
	}
	
//hlavni ciklus. V nem se spojuji nejblizsi objekty(cluster) dokud pocet clusteru se nestane == hodnote argumentu
	int c1 = 0;
	int c2 = 0;
	while(narr > arg)
	{
		find_neighbours(clusters, narr, &c1, &c2);
		merge_clusters(&clusters[c1], &clusters[c2]);
		narr = remove_cluster(clusters, narr, c2);
	}
	print_clusters(clusters, narr);

//free pameti
	for(int i = 0; i < narr; i++)
	{
		clear_cluster(&clusters[i]);
	}
	free(clusters);
	return 0;
	
}
