#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'
ORANGE='\033[0;33m'


OK="${LIGHT_GREENBG}OK${NOBG}"
KO="${LIGHT_REDBG}KO${NOBG}"
VAL=0
HITS=0
ERRORS=0
gerr=0
berr=0
cd ../builddir
check()
{
    gerr=0
    berr=0
    VAL=$((VAL+1));
    printf "Testing with args: [${ORANGE}$@${NC}]\n"
    bash -c "$@" > ref
    bash_return_code=$?
    ./42sh -c "$@" > student
    actual_return_code=$?

    if [ $actual_return_code -eq $bash_return_code ]
    then
        gerr=$((gerr+1));
    else
        berr=$((berr+1))
    fi;
    [ $bash_return_code -eq $actual_return_code ] && printf "RETURN_CODE: ${GREEN}[CLEAR]${NC}\n" || printf "RETURN_CODE:${RED}[BAD]${NC}
expected: $bash_return_code got: $actual_return_code\n"

    diff -u ref student
    output_diff=$?
    if [ $output_diff -eq 0 ]
    then
        if [ $berr -eq 0 ]
        then
            HITS=$((HITS+1));
        else
            ERRORS=$((ERRORS+1))
        fi;
    else
        ERRORS=$((ERRORS+1))
    fi;
    [ $output_diff -eq 0 ] && printf "OUTPUT: ${GREEN}[CLEAR]${NC}\n" || printf "OUTPUT: ${RED}[BAD]${NC}\n"
}


check "echo papa ; echo and ; if echo p ; then echo end ; else echo e ; fi"
check "echo -e test"
check "echo -e \"tr\\tuc\""
check "echo -e \\\\\\\\\\n"
check "echo -e"
check "echo -e \\\\\\\\\\n"
check "echo -e \\d\\g\\h\\i\\j\\k\\l\\m\\n\\o\\p\\q\\s\\t\\u\\w\\y\\z+\\d\\g\\h\\i\\j\\k\\l\\m+\\o\\p\\q\\s	\\u\\w\\y\\z"
check "echo -n -e -n -e truc\\\\\\\\"
check "echo -en hello"
check "echo -e \"\\\\\\\\\\n\""
check "echo -n inspiration"
check "echo -e - -e yop"
check "echo -e"
check "echo -n -n -n troubadour"
check "echo eezaz azeaze"
check "echo aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
check "/afs/cri.epita.fr/"
check "cd nikmok"
check "cd"
check "cd -"
check "exit 5"
check "exit"
check "exit zze"
printf "${CYAN}################# RECAPITULATIF ##################${NC}\n"

echo -e "Sur $VAL TESTS ${CYAN} ${ORANGE}Vous avez réussi${GREEN} $HITS ${ORANGE}TESTS et ratés ${RED}$ERRORS ${ORANGE}TESTS ${NC}\n "
