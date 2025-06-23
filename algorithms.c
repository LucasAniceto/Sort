#include "algorithms.h"
#include "ui.h"
#include "utils.h"

int bubble_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "BUBBLE SORT",
        "Um dos algoritmos mais simples, que compara elementos adjacentes\n     e os troca se estiverem fora de ordem.",
        "1. Percorre o array comparando pares adjacentes\n     2. Troca elementos se o primeiro for maior que o segundo\n     3. Repete até que nenhuma troca seja necessária\n     4. A cada passada, o maior elemento 'borbulha' para o final",
        "Melhor caso: O(n) - array já ordenado\n     Caso médio: O(n²)\n     Pior caso: O(n²) - array em ordem decrescente\n     Espaço: O(1) - ordenação in-place",
        "Implementação muito simples\n     Estável (mantém ordem relativa de elementos iguais)\n     Detecta quando o array já está ordenado\n     Funciona bem como ferramenta educacional",
        "Extremamente ineficiente para arrays grandes\n     Faz muitas comparações desnecessárias\n     Desempenho sempre quadrático (exceto melhor caso)",
        "Ensino de algoritmos básicos\n     Arrays muito pequenos (< 10 elementos)\n     Situações onde simplicidade é mais importante que eficiência",
        "Cocktail Sort (bidirecional)\n     Odd-Even Sort (para computação paralela)"
    );
    
    strcpy(state->status, "Iniciando Bubble Sort...");
    add_log_entry(state, "=== INÍCIO DO BUBBLE SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < state->size - i - 1; j++) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            set_status(state, "Passada %d: Comparando posições %d e %d (valores: %d e %d)", 
                    i + 1, j, j + 1, state->arr[j], state->arr[j + 1]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            if (state->arr[j] > state->arr[j + 1]) {
                reset_highlights(state);
                state->swapping[0] = j;
                state->swapping[1] = j + 1;
                
                int temp = state->arr[j];
                state->arr[j] = state->arr[j + 1];
                state->arr[j + 1] = temp;
                state->swaps++;
                swapped = 1;
                
                set_status(state, "Trocando %d com %d", state->arr[j + 1], state->arr[j]);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
        
        reset_highlights(state);
        state->sorted_until = state->size - i - 1;
        set_status(state, "Fim da passada %d. Maior elemento na posição correta.", i + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (!swapped) {
            set_status(state, "Nenhuma troca realizada. Array já está ordenado!");
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            break;
        }
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Bubble Sort concluído!");
    add_log_entry(state, "=== BUBBLE SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int selection_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "SELECTION SORT",
        "Encontra o menor elemento do array e o coloca na primeira posição,\n     depois repete para o restante do array.",
        "1. Encontra o menor elemento do array inteiro\n     2. Troca com o primeiro elemento\n     3. Encontra o menor do restante (excluindo o primeiro)\n     4. Troca com o segundo elemento\n     5. Repete até ordenar todo o array",
        "Melhor caso: O(n²)\n     Caso médio: O(n²)\n     Pior caso: O(n²)\n     Espaço: O(1) - ordenação in-place\n     Sempre faz exatamente n-1 trocas",
        "Minimiza o número de trocas (apenas n-1)\n     Desempenho consistente (sempre O(n²))\n     Simples de implementar\n     Funciona bem quando trocas são custosas",
        "Ineficiente para arrays grandes\n     Não é estável\n     Não se beneficia de dados parcialmente ordenados\n     Sempre faz O(n²) comparações",
        "Arrays pequenos onde trocas são custosas\n     Situações onde consistência de performance é importante\n     Memória limitada (usa apenas O(1) espaço extra)",
        "Heap Sort (versão otimizada usando heap)\n     Bidirectional Selection Sort"
    );
    
    strcpy(state->status, "Iniciando Selection Sort...");
    add_log_entry(state, "=== INÍCIO DO SELECTION SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int min_idx = i;
        reset_highlights(state);
        state->comparing[0] = i;
        
        set_status(state, "Procurando o menor elemento a partir da posição %d", i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        for (int j = i + 1; j < state->size; j++) {
            reset_highlights(state);
            state->comparing[0] = min_idx;
            state->comparing[1] = j;
            state->comparisons++;
            
            set_status(state, "Comparando posição %d (valor: %d) com posição %d (valor: %d)", 
                    min_idx, state->arr[min_idx], j, state->arr[j]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            if (state->arr[j] < state->arr[min_idx]) {
                min_idx = j;
                set_status(state, "Novo menor encontrado: %d na posição %d", state->arr[j], j);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
        
        if (min_idx != i) {
            reset_highlights(state);
            state->swapping[0] = i;
            state->swapping[1] = min_idx;
            
            int temp = state->arr[i];
            state->arr[i] = state->arr[min_idx];
            state->arr[min_idx] = temp;
            state->swaps++;
            
            set_status(state, "Trocando posição %d (valor: %d) com posição %d (valor: %d)", 
                    i, state->arr[min_idx], min_idx, state->arr[i]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
        }
        
        reset_highlights(state);
        state->sorted_until = i + 1;
        set_status(state, "Posição %d ordenada com valor %d", i, state->arr[i]);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Selection Sort concluído!");
    add_log_entry(state, "=== SELECTION SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int insertion_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "INSERTION SORT",
        "Constrói o array ordenado um elemento por vez, inserindo cada\n     novo elemento na posição correta entre os já ordenados.",
        "1. Considera o primeiro elemento como ordenado\n     2. Pega o próximo elemento (chave)\n     3. Compara com elementos à esquerda\n     4. Move elementos maiores para a direita\n     5. Insere a chave na posição correta\n     6. Repete até o final do array",
        "Melhor caso: O(n) - array já ordenado\n     Caso médio: O(n²)\n     Pior caso: O(n²) - array em ordem decrescente\n     Espaço: O(1) - ordenação in-place",
        "Muito eficiente para arrays pequenos\n     Excelente para dados quase ordenados\n     Estável e adaptativo\n     Ordenação online (pode ordenar conforme recebe dados)\n     Simples de implementar",
        "Ineficiente para arrays grandes\n     O(n²) comparações no pior caso\n     Muitas movimentações de elementos",
        "Arrays pequenos (< 50 elementos)\n     Dados quase ordenados ou parcialmente ordenados\n     Como sub-rotina em algoritmos híbridos (ex: Timsort)\n     Ordenação incremental de dados que chegam aos poucos",
        "Binary Insertion Sort (usa busca binária)\n     Shell Sort (versão com gaps)\n     Library Sort"
    );
    
    strcpy(state->status, "Iniciando Insertion Sort...");
    add_log_entry(state, "=== INÍCIO DO INSERTION SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 1; i < state->size; i++) {
        int key = state->arr[i];
        int j = i - 1;
        
        reset_highlights(state);
        state->comparing[0] = i;
        set_status(state, "Inserindo elemento %d (posição %d) na posição correta", key, i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        while (j >= 0 && state->arr[j] > key) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            set_status(state, "Comparando %d com %d - Movendo %d para a direita", 
                    state->arr[j], key, state->arr[j]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            state->arr[j + 1] = state->arr[j];
            state->swaps++;
            
            reset_highlights(state);
            state->swapping[0] = j;
            state->swapping[1] = j + 1;
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            j--;
        }
        
        state->arr[j + 1] = key;
        reset_highlights(state);
        state->sorted_until = i + 1;
        
        set_status(state, "Elemento %d inserido na posição %d", key, j + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Insertion Sort concluído!");
    add_log_entry(state, "=== INSERTION SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int partition(SortState *state, int low, int high, int start_y) {
    int pivot = state->arr[high];
    int i = low - 1;
    
    reset_highlights(state);
    state->comparing[0] = high; 
    set_status(state, "Pivot escolhido: %d (posição %d)", pivot, high);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int j = low; j < high; j++) {
        reset_highlights(state);
        state->comparing[0] = high;
        state->comparing[1] = j;
        state->comparisons++;
        
        set_status(state, "Comparando %d com pivot %d", state->arr[j], pivot);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (state->arr[j] < pivot) {
            i++;
            if (i != j) {
                reset_highlights(state);
                state->swapping[0] = i;
                state->swapping[1] = j;
                
                int temp = state->arr[i];
                state->arr[i] = state->arr[j];
                state->arr[j] = temp;
                state->swaps++;
                
                set_status(state, "Trocando %d (pos %d) com %d (pos %d)", 
                        state->arr[j], i, state->arr[i], j);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
    }
    
    reset_highlights(state);
    state->swapping[0] = i + 1;
    state->swapping[1] = high;
    
    int temp = state->arr[i + 1];
    state->arr[i + 1] = state->arr[high];
    state->arr[high] = temp;
    state->swaps++;
    
    set_status(state, "Colocando pivot %d na posição correta: %d", pivot, i + 1);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    reset_highlights(state);
    return i + 1;
}

int quicksort_recursive(SortState *state, int low, int high, int start_y) {
    if (low < high) {
        set_status(state, "Particionando subarray de índice %d a %d", low, high);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        int pi = partition(state, low, high, start_y);
        if (pi == -1) return -1; 
        
        set_status(state, "Partição completa. Pivot na posição %d", pi);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (quicksort_recursive(state, low, pi - 1, start_y) == -1) return -1;
        if (quicksort_recursive(state, pi + 1, high, start_y) == -1) return -1;
    }
    return 0;
}

int quicksort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "QUICK SORT",
        "Algoritmo de divisão e conquista que escolhe um pivot e particiona\n     o array: menores à esquerda, maiores à direita.",
        "1. Escolhe um elemento como pivot (geralmente o último)\n     2. Reorganiza o array: menores que pivot à esquerda, maiores à direita\n     3. Coloca o pivot na posição final correta\n     4. Aplica recursivamente nas duas partições\n     5. Combina os resultados (já estão ordenados)",
        "Melhor caso: O(n log n) - pivot sempre divide o array ao meio\n     Caso médio: O(n log n)\n     Pior caso: O(n²) - pivot sempre é o menor ou maior\n     Espaço: O(log n) - devido à recursão",
        "Muito rápido na prática (um dos mais usados)\n     Ordenação in-place\n     Divide e conquista é elegante\n     Boa localidade de cache",
        "Pior caso é O(n²)\n     Não é estável\n     Desempenho depende muito da escolha do pivot\n     Pode causar stack overflow (muita recursão)",
        "Arrays grandes (> 1000 elementos)\n     Quando espaço extra é limitado\n     Dados com distribuição aleatória\n     Implementações de bibliotecas padrão",
        "3-way QuickSort (lida melhor com elementos duplicados)\n     Randomized QuickSort (escolha aleatória do pivot)\n     Dual-Pivot QuickSort (usado no Java)"
    );
    
    strcpy(state->status, "Iniciando QuickSort...");
    add_log_entry(state, "=== INÍCIO DO QUICKSORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    if (quicksort_recursive(state, 0, state->size - 1, start_y) == -1) return -1;
    
    strcpy(state->status, "QuickSort concluído!");
    add_log_entry(state, "=== QUICKSORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}