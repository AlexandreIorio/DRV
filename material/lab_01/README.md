## Question 1
```
md.b 0x80008000 0x1
80008000: 46    F
md.w 0x80008000 0x1
80008000: 4c46    FL
md.l 0x80008000 0x1
80008000: eb004c46    FL..
md.b 0x80008000 0x4
80008000: 46 4c 00 eb    FL..
md.w 0x80008000 0x4
80008000: 4c46 eb00 9000 e10f    FL......
md.l 0x80008000 0x4
80008000: eb004c46 e10f9000 e229901a e319001f    FL........).....
```

Les lignes affichées correspondent à des commandes U-Boot pour lire des valeurs en mémoire à l'adresse `0x80008000`. Voici les différences :

**Différences entre les lignes** :
- `md.b` : lit un octet à la fois. *Pour ma part je n'ai pas le F mais 1 point*
- `md.w` : lit un mot de 2 octets. *Pour ma part je n'ai pas le FL mais 2 point.*
- `md.l` : lit une valeur de 4 octets(long). *Pour ma part je n'ai pas le FL.. mais 4 point.*
la valeur suivante, par exemple `0x1` ou `0x4`, indique le nombre d'octets, de mots ou de valeur de 4 octets (long) à lire depuis l'adresse.

voici l'usage de la commande `md` :
```sh
md <address>[<data_size>] [<length>]
```

**Accéder à une adresse non alignée** :
Acceder à une address non alignée cree un `data abort`. 

voici un exemple ou j'essaye d'accéder a un mot de 16bits a une adresse impair:

```sh
SOCFPGA_CYCLONE5 # md.w 0x80008001
80008001:data abort
``` 


Si vous essayez d'accéder à une adresse non alignée, le système pourrait déclencher une exception matérielle ou lire des données incorrectes. L'alignement est crucial pour les performances et la stabilité du système, car de nombreux processeurs s'attendent à ce que les données soient alignées sur leur taille (ex : une adresse de 4 octets alignée sur 4).

## Question 2

Voici un script qui permet faire un toggle des displays 7 segments.

```sh
setenv toggle 'mw.l 0xFF200020 0xFF000000; mw.l 0xFF200030 0x0000FFFF; sleep 1; mw.l 0xFF200020 0x00FFFFFF; mw.l 0xFF200030 0x00000000; sleep 1;'

run toggle
```

**Explication** :
- `mw.l 0xFF200020 0xFF000000` : Active l'afficheur 3
- `mw.l 0xFF200030 0x0000FFFF` : Active l'afficheur 4 et 5
- `sleep 1` : Attendre 1 seconde
- `mw.l 0xFF200020 0x00FFFFFF` : Désactive l'afficheur 3 et active l'afficheur 0,1 et 2
- `sleep 1` : Attendre 1 seconde

Dans une boucle infinie, le script alterne entre les afficheurs 0,1,2 et 3,4,5.

## Question 3

Le code a été réalisé avec des librairies qui ont été créées pour l'occasion en espérant qu'elles seront utiles pour les prochains labos. 

Pour compiler et deployer le soft, merci d'utiliser le makefile.



