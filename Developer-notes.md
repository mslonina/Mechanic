Developer notes
===============

Programming style
-----------------

- Follow C99 as close as possible, no specific compiler extensions
- 2-space indentation
- CamelCase for functions
- Doxygen-style + Markdown for documenting the API


VIM users
---------

    " Basics
    :set nocompatible
    :set nobackup
    :set ruler
    :set pastetoggle=<F11>
    :set fo=tcrqn
    :syntax on
    :filetype on
    filetype plugin on
    filetype indent on

    :autocmd FileType c,cpp :set cindent
    :set grepprg=grep\ -nH\ $*
    :set comments=s1:/**,mb:\ *,elx:*/

    " Coding standard
    :set nu
    :set textwidth=90
    :set shiftwidth=2
    :set tabstop=2
    :set smarttab
    :set expandtab
    :set autoindent
    :set smartindent
    :set makeprg=make

    " This will highlight lines longer than 80 chars
    :highlight OverLength ctermbg=red ctermfg=white guibg=#592929
    :match OverLength /\%80v.*/
    :let c_space_errors = 1 

    if has("autocmd")
      au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g`\"" | endif
    end if
