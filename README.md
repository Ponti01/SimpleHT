# Hash table in C

Piccola libreria di tabella hash generica con **concatenamento separato**: ogni bucket contiene una lista collegata di elementi. Chiavi e valori sono puntatori forniti dal chiamante; confronto e hashing sono configurabili.

Questo documento descrive il comportamento del sorgente fornito, incluse le sue limitazioni. L'header allegato aggiunge commenti in inglese e gli include `<stddef.h>` e `<stdint.h>` necessari per renderlo autonomo. Le firme pubbliche sono invariate; le correzioni suggerite di seguito **non sono state applicate all'implementazione**.

## Funzionalità

- Creazione con capacità iniziale configurabile e arrotondamento a potenza di due.
- Inserimento di nuove coppie e sostituzione del valore di una chiave equivalente.
- Ricerca e rimozione per chiave.
- Recupero del valore sostituito e della chiave rimossa tramite parametri di output opzionali.
- Accesso al numero di elementi e al numero di bucket.
- Visita delle coppie con callback e contesto utente.
- Crescita automatica con raddoppio dei bucket.
- Memorizzazione dell'hash di ogni chiave, riutilizzato durante il ridimensionamento.
- Distruzione delle sole strutture interne, senza liberare i dati dell'utente.

Non sono previsti shrinking, ordinamento, iteratori persistenti, copia dei dati, distruttori configurabili, `reserve`, `clear` o sincronizzazione interna.

## Rappresentazione e hashing

`hash_table` è un tipo opaco: i programmi client usano l'header, senza accedere ai campi interni. L'implementazione contiene un array di bucket, dimensione, capacità e due callback. Ogni nodo contiene chiave, valore, hash a 64 bit e puntatore al nodo successivo.

La capacità è sempre una potenza di due positiva. `init_cap == 0` seleziona 8 bucket; per esempio 9 viene arrotondato a 16, mentre 1 e 2 sono accettati senza imporre il minimo 8. La capacità misura i bucket, non il massimo numero di elementi.

`get_bucket_idx()` mescola l'hash tramite XOR, shift e moltiplicazioni a 64 bit, quindi seleziona il bucket con `k & (cap - 1)`. Questo passaggio redistribuisce i bit dell'hash; non può distinguere chiavi cui la callback abbia assegnato lo stesso hash. Le collisioni vengono gestite scorrendo la lista e usando `compar()`.

## Contratto delle callback e dei dati

```c
int compar(const void *a, const void *b);
uint64_t hash(const void *key);
```

`compar(a, b) == 0` indica equivalenza. Il segno degli altri risultati non è usato: non serve un ordinamento. Il confronto deve definire un'equivalenza coerente, e deve valere:

```text
compar(a, b) == 0  =>  hash(a) == hash(b)
```

L'implicazione inversa non è richiesta. Chiavi diverse possono avere lo stesso hash.

Le callback devono restare valide e coerenti per tutta la vita della tabella. I dati che determinano hash ed equivalenza delle chiavi non devono cambiare finché le chiavi sono inserite. In caso contrario, ricerca, sostituzione e rimozione possono non trovare un elemento già presente. Il ridimensionamento usa l'hash salvato e non corregge questa situazione. Le callback di hashing e confronto non devono modificare o distruggere la tabella durante un'operazione.

La tabella **non possiede chiavi e valori**: conserva gli indirizzi senza duplicare né liberare gli oggetti. Questi devono restare validi mentre sono memorizzati. Non passare indirizzi di variabili locali che escono dallo scope prima della rimozione o distruzione della tabella.

## API pubblica

### `ht_create(compar, hash, init_cap)`

Crea una tabella vuota. Restituisce `NULL` se una callback è nulla, la capacità richiesta supera il limite gestito da `next_pow2()`, oppure fallisce un'allocazione. Il limite logico `MAX_CAP` è la maggiore potenza di due rappresentabile da `size_t`; non garantisce che sia possibile allocare quell'array di puntatori.

### `ht_insert(ht, key, value, old_value)`

| Ritorno | Significato | Effetto su `*old_value`, se fornito |
| --- | --- | --- |
| `0` | Nuova coppia inserita; la dimensione aumenta | Invariato |
| `1` | Valore sostituito; la dimensione non cambia | Riceve il valore precedente |
| `-1` | `ht`, `key` o `value` nullo, oppure allocazione del nodo fallita | Invariato |

`old_value` può essere `NULL`. In caso di chiave equivalente, viene conservato **il puntatore alla chiave già presente**, non quello passato nella nuova chiamata. Il chiamante deve quindi gestire autonomamente l'eventuale nuova chiave allocata ma non memorizzata. Il vecchio valore non viene liberato.

La crescita avviene dopo l'inserimento del nodo. Se fallisce, l'inserimento resta valido e ritorna comunque `0`. Anche la sostituzione con lo stesso puntatore restituisce `1`: non liberare alla cieca `*old_value`, perché potrebbe coincidere con il valore appena conservato o essere condiviso altrove.

### `ht_get(ht, key)`

Restituisce il puntatore al valore della chiave equivalente, oppure `NULL` per chiave assente o argomenti nulli. Non restituisce una copia e non trasferisce proprietà. Poiché l'inserimento vieta valori nulli, un risultato `NULL` non rappresenta un valore memorizzato, ma non distingue assenza ed errore di argomento.

### `ht_get_size(ht)` e `ht_get_cap(ht)`

Restituiscono rispettivamente numero di elementi e numero di bucket. Con `ht == NULL` restituiscono entrambe zero. `ht_get_size() == 0` non distingue quindi una tabella vuota da un puntatore nullo.

### `ht_foreach(ht, visit, p)`

Invoca `visit(key, value, p)` per ogni elemento, in ordine non specificato. `p` viene passato invariato e può essere `NULL`. Se `ht` o `visit` è nullo, la funzione non fa nulla. Non esiste un valore di ritorno della callback per interrompere anticipatamente la visita.

I nuovi nodi vengono aggiunti in testa e il ridimensionamento ricollega le liste: non fare affidamento sull'ordine di inserimento o su un ordine stabile. Durante la visita mantenere invariata la struttura della tabella e non modificare le proprietà delle chiavi usate da hash e confronto.

### `ht_remove(ht, key, removed_key)`

Rimuove il nodo corrispondente, decrementa la dimensione e restituisce il valore. Non riduce la capacità e non libera chiave o valore.

| Esito | Ritorno | Effetto su `*removed_key`, se fornito |
| --- | --- | --- |
| Elemento trovato | Puntatore al valore | Chiave originale memorizzata |
| Elemento assente, argomenti validi | `NULL` | `NULL` |
| `ht` o `key` nullo | `NULL` | Invariato |

`removed_key` può essere `NULL`. La chiave restituita può avere indirizzo diverso dalla chiave di ricerca, purché siano equivalenti.

### `ht_destroy(ht)`

Libera nodi, array di bucket e tabella, ma non chiavi e valori. Accetta `NULL`. Dopo la chiamata, il puntatore alla tabella è invalido e non viene automaticamente azzerato; una seconda distruzione sullo stesso indirizzo è un uso non valido. I dati dell'utente non vengono invalidati dalla sola distruzione delle strutture interne.

## Crescita e complessità

Il fattore di carico è `size / cap`. Il codice tenta un raddoppio dopo ogni nuova inserzione quando `size * 4 >= cap * 3`, cioè idealmente al raggiungimento del 75%. Una sostituzione non tenta alcuna crescita. `resize()` non restituisce uno stato: capacità limite o allocazione fallita lasciano intatta la tabella esistente.

Il ridimensionamento alloca il nuovo array prima di modificare le liste, poi redistribuisce i nodi usando gli hash memorizzati. Non alloca nuovi nodi, non richiama `hash()` e non sposta gli oggetti puntati da chiavi e valori.

Indicando con `n` il numero di elementi e con `b` quello di bucket:

| Operazione | Costo |
| --- | --- |
| Creazione | O(b), nel modello usuale di inizializzazione dell'array |
| Ricerca e rimozione | O(1 + n/b) atteso con buona distribuzione; O(n) nel caso peggiore |
| Inserimento | O(1) atteso ammortizzato con crescita riuscita e buona distribuzione; O(n + b) se ridimensiona |
| Sostituzione | Come una ricerca |
| Lettura dimensione/capacità | O(1) |
| Visita, distruzione, ridimensionamento | O(n + b) |
| Memoria | O(n + b) |

I costi assumono callback a costo costante; per stringhe o chiavi complesse occorre aggiungerne il costo effettivo. Se la crescita fallisce ripetutamente, il fattore di carico può superare 0,75 e le garanzie medie peggiorano. Durante il resize convivono temporaneamente il vecchio array e quello nuovo, per un totale di `3 * b` slot di bucket.

## Esempio d'uso

Salvare l'implementazione originale in `hash_table.c`, l'header allegato in `hash_table.h` e il seguente esempio in `example.c` nella stessa directory.

```c
#include "hash_table.h"
#include <stdio.h>
#include <string.h>

static int compare_strings(const void *a, const void *b)
{
    return strcmp(a, b);
}

/* Simple deterministic string hash for this example; not cryptographic. */
static uint64_t hash_string(const void *key)
{
    const unsigned char *s = key;
    uint64_t h = UINT64_C(14695981039346656037);
    while (*s != '\0') {
        h ^= *s++;
        h *= UINT64_C(1099511628211);
    }
    return h;
}

static void print_entry(const void *key, const void *value, void *p)
{
    FILE *out = p;
    fprintf(out, "%s: %d\n", (const char *)key, *(const int *)value);
}

int main(void)
{
    int first = 10, second = 20;
    hash_table *ht = ht_create(compare_strings, hash_string, 0);
    if (ht == NULL)
        return 1;

    if (ht_insert(ht, "score", &first, NULL) != 0) {
        ht_destroy(ht);
        return 1;
    }

    const void *old = NULL;
    int status = ht_insert(ht, "score", &second, &old);
    if (status == 1)
        printf("Previous value: %d\n", *(const int *)old);

    const int *value = ht_get(ht, "score");
    if (value != NULL)
        printf("Current value: %d\n", *value);

    printf("Entries: %zu; buckets: %zu\n",
           ht_get_size(ht), ht_get_cap(ht));
    ht_foreach(ht, print_entry, stdout);

    const void *removed_key = NULL;
    const int *removed_value = ht_remove(ht, "score", &removed_key);
    if (removed_value != NULL)
        printf("Removed %s: %d\n",
               (const char *)removed_key, *removed_value);

    ht_destroy(ht);
    return 0;
}
```

```sh
cc -std=c11 -Wall -Wextra -Wpedantic hash_table.c example.c -o example
./example
```

Le variabili dell'esempio restano vive fino alla distruzione della tabella. Non si usa `free()` su stringhe letterali o variabili automatiche. Per oggetti allocati dinamicamente, predisporre una strategia di rilascio che tenga conto di alias e condivisione: non liberare due volte un oggetto usato in più coppie.

## Criticità e miglioramenti suggeriti

### 1. Overflow nel controllo del fattore di carico

`ht->size * 4` e `ht->cap * 3` possono superare `SIZE_MAX`. Il wraparound dell'aritmetica unsigned produce allora una decisione di crescita errata. Un confronto equivalente per le capacità potenze di due ammesse, incluse 1 e 2, è:

```c
if (ht->size >= ht->cap - ht->cap / 4)
    resize(ht);
```

Anche l'incremento di `size` non ha una guardia esplicita, benché le allocazioni necessarie rendano normalmente irraggiungibile quel limite. Per un contratto rigoroso, verificare `size == SIZE_MAX` prima di inserire una nuova chiave.

### 2. Capacità logica e capacità allocabile

Il controllo `MAX_CAP` protegge l'arrotondamento e il raddoppio numerico, ma non verifica esplicitamente che `new_cap * sizeof(*ht->buckets)` sia rappresentabile. Il codice passa correttamente due argomenti separati a `calloc()` e gestisce il suo fallimento; un controllo preventivo renderebbe il limite più esplicito:

```c
if (new_cap > SIZE_MAX / sizeof(*ht->buckets))
    /* Reject creation or skip growth, depending on the caller. */
```

### 3. Modifiche durante `ht_foreach()`

Salvare `e->next` prima della callback non rende la visita generalmente sicura rispetto alle modifiche. Rimuovere il nodo successivo può lasciare `next` pendente e causare accesso a memoria liberata. Un inserimento può ridimensionare e ricollegare le liste, facendo saltare o rivisitare elementi. Distruggere la tabella invalida il resto della visita.

La sola rimozione del nodo corrente è compatibile con questo specifico ciclo se il resto della struttura rimane intatto, ma non equivale a un supporto generale alle modifiche. Il contratto conservativo dell'header allegato vieta modifiche strutturali. Per supportarle formalmente servirebbe un'API o una strategia di iterazione dedicata.

### 4. Perdita di `const` nell'API

Chiavi e valori sono memorizzati come `const void *`, ma `ht_get()` e `ht_remove()` restituiscono `void *` attraverso un cast. Il cast non rende modificabile un oggetto originariamente costante; scriverci può avere comportamento indefinito. Valutare ritorni `const void *`, oppure un contratto coerentemente mutabile. Anche `new_entry->key = (void*)key` contiene un cast superfluo: basta assegnare `key`.

Le funzioni di sola lettura potrebbero accettare `const hash_table *` per esprimere meglio il contratto; ciò richiederebbe aggiornare dichiarazioni e definizioni.

### 5. Output opzionali e segnalazione degli errori

`old_value` viene scritto solo sulla sostituzione. `removed_key` viene azzerato dopo il controllo di validità degli argomenti, quindi resta invariato in caso di argomenti nulli. Inizializzare gli output in tutte le chiamate renderebbe il comportamento più uniforme, ma cambierebbe il contratto attuale documentato qui.

`ht_get()` e `ht_remove()` non distinguono chiave assente da argomenti invalidi. `ht_create()` non distingue le diverse cause del fallimento. Il fallimento della crescita non viene esposto. Queste sono scelte dell'API, non necessariamente errori, ma vanno considerate dal chiamante.

### 6. Distribuzione e resistenza alle collisioni

Il mixing finale non è una protezione contro hash scelti male o collisioni costruite appositamente. Molte chiavi nello stesso bucket possono degradare le operazioni a tempo lineare. Per input ostili valutare una funzione hash con segreto casuale; il mixing deterministico presente non sostituisce questa protezione.

L'hash salvato non viene usato per filtrare i confronti durante ricerca, inserimento e rimozione. Con callback coerenti, si può ridurre il numero di chiamate a `compar()` usando:

```c
if (curr->hash == hash && ht->compar(curr->key, key) == 0) {
    /* Matching entry. */
}
```

### 7. Memoria e concorrenza

Un'allocazione per nodo comporta overhead e accessi dispersi in memoria. La tabella non riduce la capacità dopo le rimozioni; questo evita resize frequenti ma conserva l'array più grande raggiunto.

Non ci sono lock. Letture concorrenti richiedono una tabella stabile, dati stabili e callback sicure per uso concorrente. Le operazioni con scritture concorrenti devono essere sincronizzate esternamente; lo stesso vale per modifiche ai dati dell'utente.

### 8. Header e portabilità

L'header originale non include i file che definiscono `size_t` e `uint64_t`: dipende dall'ordine degli include del client. L'header allegato risolve questo problema con `<stddef.h>` e `<stdint.h>`.

I nomi `_hash_table` e `_entry`, usati come identificatori a file scope, ricadono nello spazio riservato dell'implementazione C. Preferire tag come `struct hash_table` e `struct ht_entry`, aggiornando anche il sorgente.

Il sorgente ripete il typedef `hash_table` dopo quello dell'header. La ridefinizione dello stesso typedef allo stesso tipo è consentita in C11; per evitare problemi in C99 è sufficiente definire il corpo con `struct _hash_table { ... };`, senza ripetere il typedef. I cicli con dichiarazioni richiedono almeno C99; l'esempio usa C11 anche per questa ragione.

`uint64_t` richiede una piattaforma con un tipo intero esattamente a 64 bit. L'interfaccia non contiene guardie `extern "C"` per il collegamento da C++; aggiungerle se tale uso è previsto. `<string.h>` è incluso ma non utilizzato nell'implementazione fornita.

## Verifiche consigliate per l'implementazione

- Inserimento, ricerca, sostituzione e rimozione con chiavi equivalenti ma indirizzi diversi.
- Collisioni forzate tramite una callback hash costante; rimozione in testa, al centro e in coda.
- Capacità iniziali 0, 1, 2 e non potenze di due; conservazione degli elementi dopo più resize.
- Semantica esatta dei parametri di output su successo, assenza ed errore.
- Fallimento simulato di `malloc()` e `calloc()`, incluso il resize dopo un inserimento riuscito.
- Controllo di leak e accessi invalidi con strumenti di analisi della memoria.

La revisione descritta è basata sul sorgente fornito. Le criticità segnalate non implicano che tutti questi scenari siano stati riprodotti eseguendo la libreria.

