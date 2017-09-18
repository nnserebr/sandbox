" ------- vimrc
map ,u :source ~/.vimrc<CR>
map ,v :e ~/.vimrc<CR>

" ------- search
map ,s :.,$s///gcI<Left><Left><Left><Left><Left>
map .s :.,$s/\<\>//gcI<Left><Left><Left><Left>

map ,n :bn<CR>
map ,p :bp<CR>
" ------- color
color darkblue

" ------- status
set laststatus=2
set statusline=%f%M%R%Y%=\ \ \ %{strftime(\"%H:%M\",localtime())}\ \ %P\ %3c\ %4l/%L

" ------- save file when switching to another buffer
set autowrite

" ------- F2=save F3=close
map OQ :w<CR>
imap OQ <C-o>:w<CR>
map OR :qa<CR>
imap OR <C-o>:qa<CR>
" same for gvim
map <F2> :w<CR>
imap <F2> <C-o>:w<CR>
map <F3> :qa<CR>
imap <F3> <C-o>:qa<CR>


" ------- Alt-N -- switch to N-th buffer
map 1 :b 1<CR>
map 2 :b 2<CR>
map 3 :b 3<CR>
map 4 :b 4<CR>
map 5 :b 5<CR>
map 6 :b 6<CR>
map 7 :b 7<CR>
map 8 :b 8<CR>
map 9 :b 9<CR>
map 0 :b 0<CR>

" ------- Disable Ctrl-Z, remap to undo
map   u
imap  <C-o>u

" ------ Jumping to the previous position in file
:au BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$") | exe "normal g'\"" | endif

" ------ Syntax
syntax on

" ------ Encoding
set encoding=utf-8

" mouse
set mouse=


" ------- TODO
set backspace=2
set cindent
set cinkeys=0{,0},0),:,0#,o,O,e,!,!<S-Tab>
set cino=c1s
set ignorecase
set smartcase
set title
set restorescreen
set foldmethod=marker
set scrolloff=5
set incsearch
set hlsearch
set ww=<,>,[,]

set nowrap
set sidescroll=5
set nows
set shiftwidth=2
set softtabstop=2
set expandtab

map [17~ :cnext<CR>
map [18~ :cprevious<CR>
map [15~ :make<CR>

" toggle between .cc and .h
map ,h :e %:r.h<CR>
map ,H :e %:r.cc<CR>


" Trims trailing spaces
function! KCCConditionallyTrimTrailingSpaces ()
  :s/\s\+$//eg
endfunction

" Trim newlines at the end of certain files before saving
au BufWritePre * call KCCConditionallyTrimTrailingSpaces()





" -----  Support for editing Perforce files
" This makes editing Perforce change/client spec files by turning off tab hiliting (nolist), setting tabs up the Perforce way, and automatically moving the cursor to edit the description as soon as the file is opened.
 function! EnterPerforceFile()
      setlocal nolist noet tw=0 ts=8 sw=8 sts=8 ft=conf
        if search("<enter description here>") > 0
            normal C
                startins!
                  elseif bufname('*') != 'message'
                      /^Description:/
                          normal 2w
                            endif
                            endfunction

                            augroup filetypedetect
                            au BufNewFile,BufRead /tmp/g4_*,*p4-change*,*p4-client* call EnterPerforceFile()
    augroup END

" Vim and Perforce
command! -nargs=* -complete=file PEdit :!g4 edit %
command! -nargs=* -complete=file PRevert :!g4 revert %
command! -nargs=* -complete=file PDiff :!g4 diff %

function! s:CheckOutFile()
 if filereadable(expand("%")) && ! filewritable(expand("%"))
    let option = confirm("Readonly file, do you want to checkout from p4?"
            \, "&Yes\n&No", 1, "Question")
     if option == 1
          PEdit
     endif
     edit!
  endif
endfunction
au FileChangedRO * nested :call <SID>CheckOutFile()


" ------- Highlighting columns 81 and 82
    function! HighlightTooLongLines()
      highlight def link RightMargin Error
        if &textwidth != 0
            exec 'match RightMargin /\%<' . (&textwidth + 3) . 'v.\%>' . (&textwidth + 1) . 'v/'
              endif
              endfunction

              augroup filetypedetect
              au BufNewFile,BufRead * call HighlightTooLongLines()
    augroup END


" ------ Highlighting EOL whitespaces
function! HighlightEOLWS()
  highlight ExtraWhitespace ctermbg=red guibg=red
  exec 'match ExtraWhitespace /\s\+$/'
endfunction

augroup filetypedetect
  au BufNewFile,BufRead * call HighlightEOLWS()
augroup END



" cursor moves across lines
set ww=b,s,<,>,[,]
"set listchars=tab:\ \ ,trail:\ ,extends:»,precedes:«
set listchars=tab:\ \ ,extends:»,precedes:«


map ,q :%s/\s\+$//eg<CR>

set textwidth=0


function! VSplitIfSingleWindow()
  let n_windows = 0
  windo let n_windows = n_windows + 1
  if n_windows == 1
    82vsplit
    wincmd r
"    vsplit
  endif
endfunction

" if cul=1 the line under cursor will be blue
hi CursorLine ctermbg=Blue cterm=none

function! Gfw(window)
"  let b = bufnr('')
  call VSplitIfSingleWindow()
  wincmd t " go to top-left window
  normal mz
  let b = bufnr('')
  exe a:window . "wincmd w"
"  wincmd l
  " open b in this window
  exe "b " . b
  " jump to label z
  normal `z
  " move to the end of line and then few positions back
  normal $hhhhhh
  " go to the file:line under cursor
  normal gF
  " go the first column
  normal 0
  set cul  " make sure the line under cursor is highlighted
  wincmd t " go back to top-left window
endfun

function! Gfw1(window)
"  let b = bufnr('')
  call VSplitIfSingleWindow()
  wincmd t " go to top-left window
  normal mz
  let b = bufnr('')
  exe a:window . "wincmd w"
"  wincmd l
  " open b in this window
  exe "b " . b
  " jump to label z
  normal `z
  " move to the end of line and then few positions back
  normal $?:[0-9]hhhhhhh
  " go to the file:line under cursor
  normal gF
  " go the first column
  normal 0
  set cul  " make sure the line under cursor is highlighted
  wincmd t " go back to top-left window
endfun


nnoremap ;f :call Gfw1(2)<cr>
nnoremap   :call Gfw1(2)<cr>
nnoremap [24~ :call Gfw1(2)<cr>
nnoremap   :call Gfw1(3)<cr>

nnoremap ^C/  :call Gfw1(2)<cr>

nnoremap [24~ /== .*Possible data race\\|== Conditional\\|== Sys\\|== Invalid\\|== Use \\|Suspected atomicity\\|ERROR: AddressSanitizer\\|WARNING: ThreadSanitizer<cr>

set makeprg=make
if &term == "screen"
  set term=xterm
endif

noremap <C-K> :pyf /usr/lib/clang-format/clang-format.py<CR>
inoremap <C-K> <C-O>:pyf /usr/lib/clang-format/clang-format.py<CR>
