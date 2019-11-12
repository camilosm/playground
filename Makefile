compilar:
	@gcc main.c -lGL -lglut -lSOIL -lGLU -lGLEW -lSDL -lSDL_mixer -lm -o playground.o
	@echo "Compilado!"
run: compilar
	@echo "Executando."
	@./playground.o
	@echo "Fim."
clean:
	@rm -f *.o
	@echo "Limpo!"
