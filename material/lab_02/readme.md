# Labo 02

## Exercice 2 

**Pourquoi une région de 4096 bytes et non pas 5000 ou 10000 ? Et pourquoi on a spécifié cette adresse ?**

Nous avons alloué 0x1000 ce qui correspond à 4096 bytes. Cette taille est spéciqfique à la taille d'une page mémoire.

**Quelles sont les différences dans le comportement de `mmap()` susmentionnées ?**
Dans le contexte du framework UIO, le comportement de `mmap()` présente quelques différences importantes par rapport à son utilisation classique :

**Utilisation de l'offset** : Dans UIO, l'argument "offset" de `mmap()` indique la région de mémoire du périphérique qui doti être mappée. 
L'offset doit être un multiple de la taille d'une page mémoire `0x1000`. De manière a spécifier les différentes plage de mémoire à mapper.

**Mappage direct de la mémoire du périphérique**: Avec UIO, `mmap()` mappe directement la mémoire physique du périphérique dans l'espace d'adressage utilisateur. Dans le cadre d'une utilisation sans UIO, `mmap()` mappe la mémoire virtuelle ou des fichiers.
Ca permet un accès direct à la mémoire du périphérique qui est bien plus rapide.

**Accès sécurisé** : UIO tout de même des restrictions pour empêcher le mappage de zones de mémoire non autorisées.



## Exercice 3

**Il y a au moins 3 façons pour attendre une interruption au moyen de /dev/uio0 (voir dans le tutoriel). Lesquelles ?**

- **read()**: La fonction `read()` permet de lire les données d'un fichier. Dans le cas de UIO, elle permet de lire les interruptions du périphérique. 
- **poll()**: La fonction `poll()` permet de surveiller les changements d'état d'un fichier. Cela permet d'attendre une interruption de manière plus efficace.
- **select()**: La fonction `select()` permet de surveiller plusieurs fichiers en même temps. Elle permet d'attendre une interruption de manière plus efficace.

