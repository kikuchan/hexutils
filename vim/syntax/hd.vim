if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn match hdOthers          /[^ ]*/

syn match hdHexZero         /00/
syn match hdHexControl      /\(0[1-9a-f]\|1[0-9a-f]\)/

syn match hdHexSymbol       /\([2-6][0-9a-f]\|7[0-9a-e]\)/
"syn match hdHexAlnum       /\(20\|3[0-9]\|4[1-9a-f]\|5[0-9a]\|6[1-9a-f]\|7[0-9a]\)/
syn match hdHexHigh1        /\(7f\|[8-b][0-9a-f]\)/
syn match hdHexHigh2        /[c-f][0-9a-f]/

syn match hdAddress         /^[0-9a-f]\+:/

syn match hdAscii           /[;|].*$/

" The string literal in hd cannot be 'region'
syn match hdString          /"\(\\.\|[^"]\)*"/

hi def hdHexZero            ctermfg=244
hi def hdHexControl         ctermfg=249
hi def hdHexAlnum           ctermfg=231
hi def hdHexSymbol          ctermfg=255
hi def hdHexHigh1           ctermfg=215
hi def hdHexHigh2           ctermfg=202

hi def link hdAddress       LineNr
hi def link hdAscii         Comment
hi def link hdString        String
hi def link hdOthers        Error

let b:current_syntax = "hd"
