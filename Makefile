compilar:
	@gcc main.c -lGL -lglut -lSOIL -lGLU -lGLEW -lm -o playground.o
	@echo "Compilado!"
run: compilar
	@echo "Executando Pong."
	@./playground.o
	@echo "Fim."
clean:
	@rm -f *.o
	@echo "Limpo!"
