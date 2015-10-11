" vim -b : edit binary using hd-format
augroup Binary
  au!
  au BufReadPre  *.bin let &bin=1
  au BufReadPost * if &bin | silent %!hd
  au BufReadPost * set ft=hd | endif
  au BufWritePre * if &bin | silent %!hexdec
  au BufWritePre * endif
  au BufWritePost * if &bin | silent %!hd
  au BufWritePost * set nomod | endif
augroup END
