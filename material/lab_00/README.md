# Q1

Le programme ne compile pas en raison d' `includes` récursif, nous allons donc ajouter des include guards pour éviter cela.


# Q2

## Quel est le résultat de l’exécution ?

La sortie represente la taille de chaque type de données en octets.

## Pourquoi les tailles des tableaux `str_array` et `str_out` sont différentes ? Faites le lien avec le warning lors de la compilation.

Tout deux sont des tableaux de `char` , cependant, `str_out` étant une variable globale, sa taille est connue à la compilation et sera retournée par `sizeof(str_out)`. `str_array` étant un parametre de la fonction `print_size`, et donc `sizeof(str_array)` retournera la taille du pointeur vers le tableau, et non la taille du tableau lui-même.

## Comment pourrais-je savoir la taille du vecteur my_array ? Et s’il s’agissait d’un tableau statique ?
Pour connaitre la taille de `my_array` il est nécessaire de mémoriser sa taille ou de définir une valeur de fin permettant d'interpréter sa taille grâce au parcours du tableau. Si `my_array` était un tableau statique, il serait possible de connaitre sa taille en utilisant `sizeof(my_array)`. 

## Quelles sont les différences entre une compilation 64 et 32 bits?

La taille des pointeurs est passée de 8 octets pour une compilation 64 bits à 4 octets pour une compilation 32 bits.
On le remarque aussi sur la sortie `call result:` avec une adresse de 8 octets pour une compilation 64 bits et 4 octets pour une compilation 32 bits, respectivement. `0x596231ecf2b0` `0x5cb241b0`

En revanche, la taille des types de données reste la même.

# Q3

## Vous ne remarquez rien dans l’affichage ? Comment pouvez-vous résoudre le/les problèmes ? Que pouvez-vous en conclure sur l’endianness de votre système ?

Je remarque que l'utilisation du printf présent sur ma machine interprète le `%x` comme un entier non signé, ce qui crée une sortie peu esthétique. Cependant, on peut voir que le dernier octet de l'adresse represente l'un des octets de la valeur `0xbeefcake`. L'octet se trouvant à la première position de l'adresse est `0xfe` et l'octet se trouvant à la dernière position de l'adresse est `0xbe`. On peut donc en conclure que le système est en little-endian.

En soi ce n'est pas un problème, c'est juste une question de représentation. Si je désire retrouver la représentation de l'adresse en big-endian, je peux, soit récupérer l'adresse suivante relative a ma valeur et lire depuis le dernier octet jusqu'au premier.

```c
    ptr = (char *)&value;
    ptr += sizeof(value) - 1;

    printf("invert value\n");
    for (int i = 0; i < sizeof(value); i++)
    {
        printf("%x\n", *ptr);
        ptr --;
    }
``` 

# Q4 On part du principe qu’on ne peut pas changer la position le long de l’axe z (sinon on serait un peu embêtés avec la fonction _moveTo()) (pourquoi ???).

Etant donné que nous faisons de l'héritage de l'interface "Shape", sa méthode `moveTo` s'applique à des formes 2D et non des volumes 3D, le `#define Shape_MOVETO` ne possède pas de paramètres `newz`. Si nous pouvions changer la position le long de l'axe z, nous devrions redéfinir la méthode `moveTo` pour prendre en compte la troisieme dimension mais dans ce cas, le polymorphisme ne serait plus applicable.

# Q5 écrivez un petit logiciel pour stocker un entier et un caractère de vos choix dans les deux cas ci-dessus, et qui les affiche ensuite à l’écran.

```c 
#include <stdio.h>

/* this is a named structure we can use multllple times */
struct a {
    int b;
    char c;
};

int main() {
    struct a instance1;
    instance1.b = 42;
    instance1.c = 'A';

    printf("first case :\n");
    printf("b = %d, c = %c\n", instance1.b, instance1.c);

    /* this is an anonymous structure we can just use this instance */
    struct {
        int b;
        char c;
    } a;

    a.b = 84;
    a.c = 'Z';
    printf("second case :\n");
    printf("b = %d, c = %c\n", a.b, a.c);

    return 0;
}
```


# Q6 Regardez dans les entêtes du noyau Linux (ce ne serait pas mal de pré-filtrer la recherche avec git-grep en utilisant la bonne option… à vous de trouver laquelle… ;)) quels attributs sont utilisés et leur signification dans le manuel GCC. Résumez ensuite par écrit ce que vous avez observé.

La commande `git grep "__attribute__"` a été utilisée a la racine du dossier `linux-socfpga` et les attributs suivants ont été trouvés :

- `__attribute__((section("sec")))`
- `__attribute__((noinline))`
- `__attribute__((preserve_access_index))`
- `__attribute__((packed))`
- `__attribute__((weak))`
- `__attribute__((aligned(PAGE_SIZE)))`
- `__attribute__((aligned(8)))`

**section("section-name")** : De manière générale, le le code généré est placé dans la section `Text`. Grâce à cet attribut, nous pouvons placer une variable dans une section spécifique de la mémoire et la creer si elle n'existe pas. 

**noinline** : Cet attribut indique au compilateur de ne pas inline la fonction. De ce fait, il y aura toujours un appel de fonction plutôt qu'un code copier à l'endroit de l'appel. Cela peut être utile pour le débogage.

**preserve_access_index** : Conserve les index d'accès aux structures lors de l'utilisation des compilateurs comme `clang`. Cela permet de préserver l'interface et l'accès aux champs d'une structure, même si sa disposition est modifiée par des transformations de code. 
*Source: chatgpt*

**packed** : Cet attribut indique au compilateur de ne pas ajouter de padding entre les champs d'une structure. Cela permet de réduire la taille de la structure en mémoire, pose des problèmes s de performance.

**weak** : Cet attribut permet de déclarer une fonction ou une variable comme faible. Cela signifie que si une autre définition plus forte est trouvée, la définition faible sera ignorée.

**aligned(alignment)** : Cet attribut permet de spécifier l'alignement d'une variable. Cela peut être utile pour les variables qui doivent être alignées sur une certaine taille de mémoire. Il ovveride les effets de `-falign-functions`


# Q7 Que pouvez-vous dire au sujet de l’alignement des champs de la structure ? Répétez le test lorsque la structure est caractérisée par l’attribut packed. Pourquoi ces avertissements ? Est-ce que maintenant les adresses ont « plus de sens » ?

Je n'ai pas dû modifier le code du `#define` pour qu'il fonctionne.

voici le code:
    
```c
#include <stdio.h>
#include <stddef.h> 

#define container_of(ptr, type, member) \
    (type*)(void*)((char*)ptr - offsetof(type, member))

struct Struct {
    int a;
    char b;
    double c;
};

struct StructPacked {
    int a;
    char b;
    double c;
} __attribute((__packed__)) ;

int main() {
    struct Struct s;

    // get addresses of the members
    int *pa = &s.a;
    char *pb = &s.b;
    double *pc = &s.c;

    // Use container_of to get the address of the structure
    struct Struct *from_a = container_of(pa, struct Struct, a);
    struct Struct *from_b = container_of(pb, struct Struct, b);
    struct Struct *from_c = container_of(pc, struct Struct, c);

    struct StructPacked s_packed;

    int *pa_packed = &s_packed.a;
    char *pb_packed = &s_packed.b;
    double *pc_packed = &s_packed.c;

    struct StructPacked *from_a_packed = container_of(pa_packed, struct StructPacked, a);
    struct StructPacked *from_b_packed = container_of(pb_packed, struct StructPacked, b);
    struct StructPacked *from_c_packed = container_of(pc_packed, struct StructPacked, c);

    printf("Origina address of struct: \t %p\n", &s);
    printf("Address from a:\t\t\t %p\n", from_a);
    printf("Address from b:\t\t\t %p\n", from_b);
    printf("Address from c:\t\t\t %p\n", from_c);
    printf("Offset of a: %ld\n", offsetof(struct Struct, a));
    printf("Offset of b: %ld\n", offsetof(struct Struct, b));
    printf("Offset of c: %ld\n", offsetof(struct Struct, c));

    printf("Origina address of struct packed:%p\n", &s_packed);
    printf("Address from a packed:\t\t %p\n", from_a_packed);
    printf("Address from b packed:\t\t %p\n", from_b_packed);
    printf("Address from c packed:\t\t %p\n", from_c_packed);
    printf("Offset of a packed: %ld\n", offsetof(struct StructPacked, a));
    printf("Offset of b packed: %ld\n", offsetof(struct StructPacked, b));
    printf("Offset of c packed: %ld\n", offsetof(struct StructPacked, c));

    return 0;
}
```

voici le résultat:

```
> ./container_of                    
Origina address of struct:       0x7ffecf0af540
Address from a:                  0x7ffecf0af540
Address from b:                  0x7ffecf0af540
Address from c:                  0x7ffecf0af540
Offset of a: 0
Offset of b: 4
Offset of c: 8
Origina address of struct packed:0x7ffecf0af533
Address from a packed:           0x7ffecf0af533
Address from b packed:           0x7ffecf0af533
Address from c packed:           0x7ffecf0af533
Offset of a packed: 0
Offset of b packed: 4
Offset of c packed: 5
```


# Q9 Écrivez une fonction dans GodBolt pour voir par vous-mêmes ce que cela veut dire en termes de code assembleur — c.-à-d., écrivez une fonction qui fait le test susmentionné avec et sans le keyword volatile, et avec différents niveaux d’optimisation (essayez p. ex., avec -O0 et -O3). Commentez par écrit les résultats.


avec ce code:

```c
#include <stdint.h>
#define BIT(nr)         (1UL << (nr))

int le_test_susmentionne(void) {
    uint32_t x = 0;
    uint32_t const n = 2;
    uint32_t b;
    // test bit n value
    b = (x >> n) & 0x1;
    // set bit n
    x |= BIT(n);
    // clear bit n
    x &= ~BIT(n);
    // toggle bit n
    x ^= BIT(n);
}
```

Compilation avec -O0:

```asm
le_test_susmentionne:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], 0
        mov     DWORD PTR [rbp-8], 2
        mov     eax, DWORD PTR [rbp-8]
        mov     edx, DWORD PTR [rbp-4]
        mov     ecx, eax
        shr     edx, cl
        mov     eax, edx
        and     eax, 1
        mov     DWORD PTR [rbp-12], eax
        mov     eax, DWORD PTR [rbp-8]
        mov     edx, 1
        mov     ecx, eax
        sal     rdx, cl
        mov     rax, rdx
        or      DWORD PTR [rbp-4], eax
        mov     eax, DWORD PTR [rbp-8]
        mov     edx, 1
        mov     ecx, eax
        sal     rdx, cl
        mov     rax, rdx
        not     eax
        and     DWORD PTR [rbp-4], eax
        mov     eax, DWORD PTR [rbp-8]
        mov     edx, 1
        mov     ecx, eax
        sal     rdx, cl
        mov     rax, rdx
        xor     DWORD PTR [rbp-4], eax
        nop
        pop     rbp
        ret
```

Compilation avec -O3:

```asm
le_test_susmentionne:
        ret
```

On remarque que le compilateur a optimisé le code en retirant les instructions inutiles, c'est à dire tout car la valeur de retour n'est jamais utilisée.

ajoutons le keyword `volatile` à la variable `x`:

```c
#include <stdint.h>
#define BIT(nr)         (1UL << (nr))

int le_test_susmentionne(void) {
    volatile uint32_t x = 0;
    uint32_t const n = 2;
    uint32_t b;
    // test bit n value
    b = (x >> n) & 0x1;
    // set bit n
    x |= BIT(n);
    // clear bit n
    x &= ~BIT(n);
    // toggle bit n
    x ^= BIT(n);
}
```

Avec -O0, nous avons, comme précédemment, la totalité des instructions.
Avec -O3, nous avons maintenant:

```asm
le_test_susmentionne:
        mov     DWORD PTR [rsp-4], 0
        mov     eax, DWORD PTR [rsp-4]
        mov     eax, DWORD PTR [rsp-4]
        or      eax, 4
        mov     DWORD PTR [rsp-4], eax
        mov     eax, DWORD PTR [rsp-4]
        and     eax, -5
        mov     DWORD PTR [rsp-4], eax
        mov     eax, DWORD PTR [rsp-4]
        xor     eax, 4
        mov     DWORD PTR [rsp-4], eax
        ret
```

on remarque que le compilateur a optimisé le code en retirant les instructions inutiles sauf celle ou la variable `x` est modifiée ou lue.


# Q10 Écrivez un petit logiciel qui déclare une variable entière sur 32 bits, ensuite affectez-lui une valeur. Effectuez un shift vers la gauche d’un nombre fixe de bits > 32, par exemple 42, et exécutez le logiciel. Qu’est-ce qui se passe ? Est-ce que le compilateur vous donnes un avertissement ou pas ? Et si le nombre de bits à shifter était donné par une variable rentrée par l’utilisateur ? Regardez dans GodBolt comment le code assembleur change avec le code et selon le niveau d’optimisation. Essayez ensuite de compiler avez UBSAN activé, observez le comportement du système et commentez-le.

Avec ce code:

```c
#include <stdio.h>
#include <stdint.h> 

int main() {

    uint32_t value = 1;
    int shift = 42; 
    uint32_t result = value << shift;

    printf("Result of %u left shifts bits of %d is : %d\n", shift,value , result);
    return 0;
}
```

le résultat est:

```
Result of 42 left shifts bits of 1 is : 1024
```

ce qui correspond à un shift de 10 bits et non 42. Le compilateur ne donne pas d'avertissement.

avec un code qui demande le nombre de bits à shifter:

```c
#include <stdio.h>
#include <stdint.h>

int main() {
    uint32_t value = 1;
    int shift;

    printf("Enter the number of bits to shift: ");
    int read  = scanf("%d", &shift);

    uint32_t result = value << shift;

    printf("Result of %u left shifts bits of %d is : %d\n", shift,value , result);

    return 0;
}
```

Le compilateur ne donne toujours pas d'avertissement relatif à un shift de plus de 32 bits.

Mais en jouant avec les valeurs d'entrée on s'aperçoit que le shift correspond à `shift % 32` avec 32 étant le nombre de bits de l'entier.

Si l'on compile avec UBSAN activé, lors de l'exécution du programme, on obtient le message suivant:

```
ubsan_user.c:12:29: runtime error: shift exponent 42 is too large for 32-bit type 'unsigned int'
```
