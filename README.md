# **Monitor de Hardware** 

Programa usado para ler e interpretar todo o sistema do seu computador, utilizando a Windows API.
Componentes a serem lidos e dados extraídos:

1. O conceito base (lendo a memória ram)

   A maneira mais fácil de começar é monitorando a memória RAM. No Windows usa-se a biblioteca nativa `<windows.h>` e uma função chamada `GlobalMemoryStatusEx`. Ela preenche uma estrutura de dados com informações detalhadas sobre a memória física e virtual do seu computador.



**Código inicial:**


```
#include <windows.h>
#include <stdio.h>

int main() {
    // 1. Cria a estrutura que vai receber os dados e define o tamanho dela
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    // 2. Chama a função da API do Windows
    if (GlobalMemoryStatusEx(&memInfo)) {
        
        // Os valores vêm em Bytes. Vamos converter para Gigabytes (GB)
        // 1 GB = 1024 * 1024 * 1024 bytes
        DWORDLONG totalPhysMem = memInfo.ullTotalPhys / (1024 * 1024 * 1024);
        DWORDLONG availPhysMem = memInfo.ullAvailPhys / (1024 * 1024 * 1024);
        DWORDLONG physMemUsed = totalPhysMem - availPhysMem;

        // 3. Exibe os resultados no terminal
        printf("=== Monitor de Hardware ===\n\n");
        printf("Carga da Memoria: %d %%\n", memInfo.dwMemoryLoad);
        printf("Memoria Total:    %llu GB\n", totalPhysMem);
        printf("Memoria em Uso:   %llu GB\n", physMemUsed);
        printf("Memoria Livre:    %llu GB\n", availPhysMem);
        
    } else {
        printf("Erro ao tentar ler as informacoes de memoria.\n");
    }

    return 0;
}
```


2. Lendo o uso da CPU

   Ler a CPU em C no Windows exige um pouco mais de matemática. O processador não diz "estou em 40%". Ele te dá o tempo total que passou trabalhando e o tempo total que passou ocioso.

   Para implementar isso no projeto, eu vou precisar:

   -  pesquisar a função `GetSystemTimes()` da `<windows.h>` .
   - Fazer duas leituras com um intervalo de tempo entre elas (ex: usar a função `Sleep(1000)` para pausar por 1 seg).
   - Subtrair os tempos da segunda leitura pelos da primeira leitura para descobrir quanto tempo a CPU passou trabalhando naquele último segundo


3. Como evoluir esse projeto?

   Depois que o básico estiver funcionando, vou transformar esse script numa ferramenta mais robusta:

     - **Criar um loop de atualização:** colocar o código dentro de um `while(1)` . Usar comando `system("cls")` do `<stdlib.h>` no início do loop e um `Sleep(1000)` no final. Isso vai fazer com que o terminal atualize os dados em tempo real, parecendo um monitor real.
     
	 - **Barras de progresso em ASCII:** em vez de mostrar apenas `60%` , vou programar um laço `for` que exiba caracteres na tela para criar uma barra visual, como [█████░░░░].
	 
	 - **Alertas visuais:** vou adicionar regras simples (um bloco `if/else` ) para mudar a cor do texto no terminal caso o uso da memória passe de 85%, simulando um alerta de gargalo no sistema
