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

Expliquer

## Question 2

```sh
setenv toggle_hex 'mw.l 0xFF200020 0x00000000; mw.l 0xFF200030 0x0000FFFF; sleep 1; mw.l 0xFF200020 0x00FFFFFF; mw.l 0xFF200030 0x00000000; sleep 1;'
```
