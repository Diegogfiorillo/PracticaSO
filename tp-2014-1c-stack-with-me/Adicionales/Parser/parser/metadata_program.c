char* _separarLineas(char*);
void _agregarEtiqueta(char*, t_metadata_program*, char*);

t_metadata_program* metadata_desde_literal(const char* literal){
	char* copia_literal = strdup(literal);	//Para conservar el const-ness
	t_metadata_program* ret = malloc( sizeof(t_metadata_program) );

	memset(ret, 0, sizeof(t_metadata_program) );
	char* buffer;

	int position = 0;			//Bytes de offset en el copia_literal
	int tamanioTotal = strlen(copia_literal);

	_separarLineas(copia_literal);
	while( position<tamanioTotal ){
		buffer = copia_literal+position;
		t_intructions auxinst = { .start = position, .offset = strlen(buffer) +1 };
		buffer = _string_trim(buffer);

		if( string_starts_with(buffer, TEXT_BEGIN) ){
			ret->instruccion_inicio = ret->instrucciones_size;
		} else {
			if(	string_starts_with(buffer, TEXT_START_LABEL) ){
				_agregarEtiqueta(buffer, ret, TEXT_START_LABEL);
				ret->cantidad_de_etiquetas++;
			} else if( string_starts_with(buffer, TEXT_FUNCTION) ){
				_agregarEtiqueta(buffer, ret, TEXT_FUNCTION);
				ret->cantidad_de_funciones++;
			} else if( !(buffer[0] == TEXT_COMMENT || buffer[0] == '\0') ) {
					ret->instrucciones_size++;
					ret->instrucciones_serializado = realloc( ret->instrucciones_serializado, sizeof(t_intructions) * ret->instrucciones_size );
					memcpy( ret->instrucciones_serializado + ret->instrucciones_size - 1, &auxinst, sizeof(t_intructions) );	//La pone en el programa
			}
		}
		position += auxinst.offset;
	}
	free(copia_literal);
	return ret;
}

t_metadata_program* metadatada_desde_literal(const char* literal){
	puts("DEPRECATED: la funcion metadatada_desde_literal tenia un error de tipeo y esta deprecada.");
	puts("DEPRECATED: deberia llamarse a la funcion metadata_desde_literal en cambio.");
	return metadata_desde_literal(literal);
}

void metadata_destruir(t_metadata_program* victima){
	free(victima->etiquetas);
	free(victima->instrucciones_serializado);
	free(victima);
}

t_puntero_instruccion metadata_buscar_etiqueta(const t_nombre_etiqueta objetivo, const char *etiquetas, const t_size etiquetas_size) {

	int offset = 0;

	while(offset < etiquetas_size){
	 	const char* nombre = etiquetas + offset;
	 	if( strncasecmp(nombre, objetivo, etiquetas_size - offset) == 0 )
	 			return *(nombre + strlen(nombre) + 1);
		offset += strlen(nombre) + 1 + sizeof(t_puntero_instruccion);
	}
	return -1;
}


// **** FUNCIONES AUXILIARES ****


char* _separarLineas(char* linea){
	int i;
	for(i=0; linea[i] != '\0'; i++){
		if( linea[i] == '\n' ){
			linea[i] = '\0';
		}
	}
	return linea;
}

void _agregarEtiqueta(char* linea, t_metadata_program* programa, char* prefix){
	char* auxName = linea + strlen(prefix) ;
	int etiquetaNameLength = (strlen(auxName) +1) * sizeof(char);

	programa->etiquetas = realloc(programa->etiquetas, programa->etiquetas_size + etiquetaNameLength + sizeof(t_puntero_instruccion));
	memcpy(programa->etiquetas+programa->etiquetas_size, auxName, etiquetaNameLength );
	memcpy(programa->etiquetas+programa->etiquetas_size+etiquetaNameLength, &programa->instrucciones_size, sizeof(t_puntero_instruccion) );

	programa->etiquetas_size += etiquetaNameLength+sizeof(t_puntero_instruccion);
}

