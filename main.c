/**
 * Kostra programu pro 3. projekt IZP 2015/16
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 * http://is.muni.cz/th/172767/fi_b/5739129/web/web/slsrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

/*****************************************************************
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

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct objt_t - struktura objektu: identifikator a souradnice
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

/*****************************************************************
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
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

//urcime si velkost objektov,vypocet velkosti clustera , pocet objektov *cap
    size_t size = sizeof(struct obj_t) * cap;
//alokacia pamate
    c->obj = malloc(size);
    assert(c->obj != 0); //testujem malloc
    c->size = 0;
    c->capacity = cap;
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
// odstranime postupne vsetky objekty z clustera, teda uvolnenie pameti
    free(c->obj);
    c->obj = NULL;
    c->capacity = 0;
    c->size = 0;
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

    c->obj = arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
// ak sa velkost clustera rovna velkosti kapacity, resize_cluster zvacsi aktualnu kapacitu clustera o cluster_chunk (10)
    if (c->size == c->capacity) {
        c = resize_cluster(c, c->capacity + CLUSTER_CHUNK);
    }
// zapise hodnotu pridaneho objektu na koniec shluku 'c'
    c->obj[c->size].id = obj.id;
    c->obj[c->size].x = obj.x;
    c->obj[c->size].y = obj.y;
    c->size++;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);
//dokym je ten mensi cluster 2 tak
//postupne pridavanie objektov z c2 do c1
    for (int i = 0; i < c2->size; i++) {
    	// c1 je v pripade potreby zvacsene rozsirene cez append
        append_cluster(c1, c2->obj[i]);
    }

    sort_cluster(c1);
}

/**********************************************************************/
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
// odstranenie shluku na danom idx
    clear_cluster(&carr[idx]);
// dokym nenarazim na idx zmazem prvok
//posun ostatnych shlukov o jedno miesto dolava
    for (int i = idx; i < narr; i++) {
        if (i == narr-1)
            carr[i] = carr[i];
        else
            carr[i] = carr[i+1];
    }
// odratanie prvokv
// novy pocet shlukov je v poli o 1 mensi
    return --narr;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
	//pytagorova veta
    assert(o1 != NULL);
    assert(o2 != NULL);
    float distance = 0;
    float x = o1->x - o2->x;
    //pocitam mocninou
    x *= x; //mocnim
    float y = o1->y - o2->y;
    y *= y; //mocnim
    distance = sqrtf (x + y);
    //vraciam vzdialenost
    return distance;
}

/*
 Pocita vzdalenost dvou shluku. Vzdalenost je vypoctena na zaklade nejblizsiho
 souseda.
 */
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);
//urcujem ci je prvy objekt min
//vzdialenost medzi dvoma prvymi obj. dvoch clusterov
    float cluster_distance = obj_distance(&(c1->obj)[0], &(c2->obj)[0]);
    for (int i = 0; i < c1->size; i++)      //porovnam kazdy objekt clusteru
        {
        for (int j = 0; j < c1->size; j++)
        {
            float distance = obj_distance(&(c1->obj)[i], &(c2->obj)[j]);
            if (distance < cluster_distance)
            {
                cluster_distance = distance;
            }
        }
    }
    return cluster_distance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky (podle nejblizsiho souseda). Nalezene shluky
 identifikuje jejich indexy v poli 'carr'. Funkce nalezene shluky (indexy do
 pole 'carr') uklada do pameti na adresu 'c1' resp. 'c2'.
 */
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);
//hladam susedne clustery
//vzdialenost prvych dvoch clusterov v poli carr
    float min_distance = cluster_distance(&(carr)[0],&(carr)[1]);
//hladam najblizsich susedov
    for (int i = 0; i < narr; i++)
    {
        for (int j = i+1; j < narr; j++)
            {
            float distance = cluster_distance(&(carr)[i],&(carr)[j]);
            if (distance < min_distance)
                {
                *c1 = i;
                *c2 = j;
                min_distance = distance;
                }
            }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = a;
    const struct obj_t *o2 = b;
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

    FILE * fp;
    int count = 0;
    // otvori neprazdny subor
    fp = fopen(filename, "r");
    if (fp == NULL)
        {
        exit(EXIT_FAILURE);
        }

    fscanf(fp,"count=%d",&count);
//vytvorim cluster, ulozim do pola clusterov
    struct cluster_t *clusters = NULL;
//alokujem pamet pre pole
    clusters = malloc(count * sizeof(struct cluster_t));
//nacitam objekty do clusteru
    int i;
    for (i = 0; i < count; i++)
        {
//inicialiyujem cluster, nastavujem kapacitu
        init_cluster(&clusters[i], CLUSTER_CHUNK);
        struct obj_t object;
//nacitavam parametre objektu
        fscanf(fp, "%d %f %f", &object.id, &object.x, &object.y);
//pridam objekty na koniec clusterov
        append_cluster(&clusters[i], object);
        }

*arr = clusters;
fclose(fp);
return i;
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
    struct cluster_t *clusters;
//nacitane objekty, pocet
    int narr = load_clusters(argv[1], &clusters);
//ak nie je pocet shlukov, defaultne 1 nastavena
    long n;
    if (argc < 3)
        {
        n = 1;
        } else
        {
        char * endptr;
        n = strtol(argv[2], &endptr, 10);
        }

    int c1, c2;

    while (n < narr)
        {
        find_neighbours(clusters, narr, &c1, &c2);
        //hladanie clustrov a ak bude prvy mensi ako druhy spojim c1 z c2
        if (c1 < c2)
        {
              //spoji sa prvy s druhym
            merge_clusters(&(clusters)[c1], &(clusters)[c2]);
            //po pridani vymazavam objekty
            narr = remove_cluster(clusters, narr, c2);
        } else
            {
            merge_clusters(&(clusters)[c2], &(clusters)[c1]);
            narr = remove_cluster(clusters, narr, c1);
            }
        }

    print_clusters(clusters, narr);

    for (int i = 0; i < narr; i++)
        {
//dealokujem pamat
        clear_cluster(&clusters[i]);
        }

    clear_cluster(&clusters[0]);
    free(clusters);
    return 0;
}
