PORT=3389
SRC=defender.c
.PHONY:clean test check
3389def: $(SRC)
	$(CC) -Wall -DLISTENPORT=$(PORT) -o $@ $^ -static

test: $(SRC)
	gcc -g -Wall -DLISTENPORT=$(PORT) -D__TEST__ -o $@ $^

clean:
	rm -rf 3389def test record

check:
	splint -Dsocklen_t=int +trytorecover -warnposix +unix-lib +skip-sysheaders $(SRC)
