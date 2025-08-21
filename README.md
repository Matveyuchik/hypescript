## HypeScript

Интерпретатор динамически типизируемого языка HypeScript с русскоязычными ключевыми словами и удобными встроенными функциями. Репозиторий содержит:
- CLI-интерпретатор `hypescript` (C11)
- Примеры (`examples/`)
- VS Code-расширение для подсветки и сниппетов (`vscode/`)

### Возможности
- Ключевые слова: `!HYPE!`, `esli`/`inache`, `poka`, `dlya`, `slomat`, `prodolzhit`
- Встроенные: `pechat(...)`, `vhod(prompt?)`, `son(ms)`, `chislo(x)`, `stroka(x)`, `logika(x)`
- Литералы: `istina`, `lozh`, `NICHTO`
- Функции пользователя: `prikol name(arg1, arg2) { ... }`
- «Указатели»: `ukazatel("name")`, `znach(ptr)`, `prisvoit(ptr, value)`

### Сборка и запуск
```bash
make
./hypescript examples/hello.hype
```
Установка (суперпользователь):
```bash
sudo make install PREFIX=/usr
# удалить: sudo make uninstall PREFIX=/usr
```

### Пример кода
```hype
!HYPE!
pechat("Hello, HypeScript!");
name = vhod("Your name: ");
pechat("Privet, "+name+"!");

esli (name == "Hype") {
  pechat("You're hyped!");
} inache {
  pechat("Stay hyped!");
}

// циклы
i = 0;
poka (i < 3) { pechat("i=", i); i = i + 1; }

dlya (j = 0; j < 3; j = j + 1) {
  esli (j == 1) { prodolzhit; }
  esli (j == 2) { slomat; }
  pechat("j=", j);
}

// преобразования и сон
pechat(stroka(123) + " -> number:", chislo("456"));
pechat("bool of NICHTO:", logika(NICHTO));
pechat("sleeping 500ms..."); son(500); pechat("awake!");

// функции и «указатели»
prikol hello(who) { pechat("Privet,", who); }
hello(name);

x = 10; p = ukazatel("x");
pechat("x before:", znach(p));
prisvoit(p, 42);
pechat("x after:", x);
```

### VS Code-расширение
Сборка и локальная установка VSIX:
```bash
cd vscode
npx --yes @vscode/vsce package
# затем установите в VS Code через: Extensions → «Install from VSIX…»
```

### Пакет для Linux
- Быстрая установка: `sudo make install PREFIX=/usr`
- Manjaro/Arch: можно собрать пакет через `makepkg` или собрать .deb/.rpm с помощью `fpm`.

### Лицензия
Добавьте файл `LICENSE` (например, MIT). Код примеров — публично доступен для использования.
