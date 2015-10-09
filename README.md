# hexutils
Comes with **hd** and **hexdec**

## SYNOPSIS
### hd
A ordinary yet another **hexdump** tool.

```
Usage: hd [options] [file ...]
  General
    -c: hexdump mode (default)
    -q: suppress chars in hexdump mode
    -f <N>: fold each N chars. (unlimited = 0, default = 16)
            (0 is not supported on hexdump mode)
    -h: this help

  Pre-defined hexencode mode
    -S: print hex in simple style   ; equiv. to -X '%02x' -G ' '
    -s: print hex w/ space prefixed ; equiv. to -X ' %02x'
    -u: print url encoded           ; equiv. to -X '%%%02x'
    -x: print hex in C style        ; equiv. to -X '0x%02x' -G ', ' -F ',\n'
    -l: print hex in lisp style     ; equiv. to -X '#x%02x' -G ' '

  Design hexencode format by hand
    -X <fmt>: set HEX style in printf format
    -G <str>: set GAP string between hex octets
    -H <str>: set HEADER string for the beginning
    -T <str>: set TAIL string for the end
    -F <str>: set separator for each FOLD (default to "{TAIL}\n{HEADER}")
```

There are two modes. **hexdump mode** and **hexencode mode**.

### hexdec
A yet another hex(dump) **reverser**.
```
Usage: hexdec [options] [file ...]
  -h: this help
```

It convert sequence of hex digits (or `hd` hexdump format) to binary data.

Additionally, it also supports **string literal injection**.
You can write ASCII string directly among hex digits.

It *ignores* address part of hexdump format to be convenient to forge binary data with editors such as vim.

## EXAMPLES
### hd (hexdump mode)
```
% hd hd | head -5
00000000: 7f 45 4c 46 02 01 01 09  00 00 00 00 00 00 00 00  |.ELF............|
00000010: 02 00 3e 00 01 00 00 00  c0 09 40 00 00 00 00 00  |..>.......@.....|
00000020: 40 00 00 00 00 00 00 00  30 1a 00 00 00 00 00 00  |@.......0.......|
00000030: 00 00 00 00 40 00 38 00  08 00 40 00 1d 00 1a 00  |....@.8...@.....|
00000040: 06 00 00 00 05 00 00 00  40 00 00 00 00 00 00 00  |........@.......|
```
### hd (hexencode mode)
```
% echo "日本語の文字列" | hd -u
%e6%97%a5%e6%9c%ac%e8%aa%9e%e3%81%ae%e6%96%87%e5
%ad%97%e5%88%97%0a
```
```
% echo "日本語の文字列" | hd -x
0xe6, 0x97, 0xa5, 0xe6, 0x9c, 0xac, 0xe8, 0xaa, 0x9e, 0xe3, 0x81, 0xae, 0xe6, 0x96, 0x87, 0xe5,
0xad, 0x97, 0xe5, 0x88, 0x97, 0x0a
```
### hexdec
```
% hd hd | hexdec | diff -s - hd
Files - and hd are identical
```
```
% echo 'e6 97 a5 e6 9c ac e8 aa 9e e3 81 ae e6 96 87 e5 ad 97 e5 88 97' | hexdec
日本語の文字列
```
Using **string literal injection**
```
% echo '"Japanese language de "             e6 96 87 e5 ad 97 e5 88 97' | hexdec
Japanese language de 文字列
```

### Combined with vim
These tools can be combined with `vim` to simulate ancient JVim3 (Japanized Vim 3) binary mode behavior.
```
" vim -b
augroup Binary
	au!
	au BufReadPre  *.bin let &bin=1
	au BufReadPost * if &bin | silent %!hd
	au BufReadPost * set ft=xxd | endif
	au BufWritePre * if &bin | silent %!hexdec
	au BufWritePre * endif
	au BufWritePost * if &bin | silent %!hd
	au BufWritePost * set nomod | endif
augroup END
```
