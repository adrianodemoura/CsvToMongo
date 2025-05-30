# CSV para MongoDB

Este projeto é um script em C para importar arquivos CSV para o MongoDB, com suporte a múltiplas threads e controle de recursos.

## Características

- Importação de múltiplos arquivos CSV em paralelo (cada arquivo é processado por uma thread independente)
- Controle de uso de memória RAM (configurável)
- Controle de número de threads (configurável)
- Estrutura de dados otimizada para MongoDB
- Sistema de logs detalhado
- Configuração flexível via arquivo JSON
- Suporte a campos aninhados (emails e telefones como subcoleções)
- Validação de campos conforme arquivo fields.txt
- Mapeamento de campos via arquivo JSON

## Requisitos

- GCC 7.0 ou superior
- libmongoc 1.0 ou superior
- libbson 1.0 ou superior
- libjson-c
- pkg-config

## Instalação

1. Instale as dependências:

```bash
sudo apt-get update
sudo apt-get install -y \
    libmongoc-dev \
    libbson-dev \
    libjson-c-dev \
    pkg-config
```

2. Clone o repositório:

```bash
git clone https://github.com/seu-usuario/csv-to-mongo.git
cd csv-to-mongo
```

3. Compile o projeto:

```bash
make clean
make
```

## Configuração

O arquivo `config/config.json` contém as configurações do projeto:

```json
{
    "mongodb_host": "127.0.0.1",
    "mongodb_port": 27017,
    "mongodb_database": "",
    "mongodb_collection": "",
    "mongodb_username": "",
    "mongodb_password": ""
}
```

### Campos de Configuração

- `mongodb_host`: Host do MongoDB
- `mongodb_port`: Porta do MongoDB
- `mongodb_database`: Nome do banco de dados
- `mongodb_collection`: Nome da coleção
- `mongodb_username`: Usuário do MongoDB
- `mongodb_password`: Senha do MongoDB

## Uso

1. Coloque seus arquivos CSV no diretório `files_csv/` (os arquivos devem seguir o padrão `pagina_NNNN.csv`)
2. Certifique-se que o arquivo `fields.txt` na raiz do projeto contém a lista de campos válidos
3. Configure o mapeamento de campos no arquivo `config/field_mapping.json`
4. Execute o script:

```bash
./bin/csv_to_mongo
```

## Estrutura do Projeto

```
.
├── config/
│   ├── config.json           # Configurações do projeto
│   ├── config_loader.c       # Carregador de configurações
│   ├── config_loader.h       # Header do carregador
│   └── field_mapping.json    # Mapeamento de campos
├── mongodb/
│   ├── mongodb_client.c      # Cliente MongoDB
│   └── mongodb_client.h      # Header do cliente
├── utils/
│   ├── memory_manager.c      # Gerenciador de memória
│   ├── memory_manager.h      # Header do gerenciador
│   ├── logger.c              # Sistema de logs
│   ├── logger.h              # Header do logger
│   ├── string_utils.c        # Utilitários de string
│   └── string_utils.h        # Header dos utilitários
├── src/
│   └── main.c               # Ponto de entrada do programa
├── files_csv/               # Diretório para arquivos CSV
├── fields.txt               # Lista de campos válidos
├── Makefile                # Script de compilação
└── README.md               # Este arquivo
```

## Estrutura dos Dados

O script importa os dados para o MongoDB com a seguinte estrutura:

```json
{
    "cpf": "123.456.789-00",
    "nome": "João da Silva",
    "nasc": "01/01/1980",
    "renda": "5000.00",
    "affinity_score": "0.85",
    "affinity_percent": "85",
    "sexo": "M",
    "cbo": "123456",
    "mae": "Maria da Silva",
    "data_atualizacao": "20240320",
    "nota": "5",
    "banco": "001",
    "cpf_conjuge": "987.654.321-00",
    "serv_publico": "N",
    "data_obito": null,
    "cidade": "Belo Horizonte",
    "endereco": "Rua Exemplo, 123",
    "bairro": "Centro",
    "cep": "31234-567",
    "uf": "MG",
    "contatos": {
        "telefones": [
            "31999999999",
            "31988888888",
            "31977777777"
        ],
        "emails": [
            "joao@email.com",
            "joao.silva@email.com"
        ]
    }
}
```

## Mapeamento de Campos

O arquivo `config/field_mapping.json` define como os campos do CSV são mapeados para o MongoDB:

```json
{
    "fields": {
        "cpf": 2,
        "nome": 3,
        "nasc": 4,
        "renda": 5,
        "affinity_score": 6,
        "affinity_percent": 7,
        "sexo": 9,
        "cbo": 10,
        "mae": 11,
        "data_atualizacao": 12,
        "nota": 12,
        "banco": 13,
        "cpf_conjuge": 14,
        "serv_publico": 15,
        "data_obito": 16,
        "cidade": 17,
        "endereco": 18,
        "bairro": 19,
        "cep": 20,
        "uf": 21
    },
    "contatos": {
        "telefones": [22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35],
        "emails": [36]
    }
}
```

## Logs

Os logs são salvos no arquivo `import.log` e incluem:
- Início e fim da importação
- Progresso da importação (a cada 1000 registros)
- Erros e avisos
- Uso de memória
- Status de cada arquivo processado

## Limitações

- Campos de email e telefone são agrupados em uma subcoleção `contatos`
- Os arquivos CSV devem seguir o padrão `pagina_NNNN.csv`
- O número máximo de threads é limitado a 16 (definido no código fonte como `MAX_THREADS`). Cada thread processa um arquivo CSV independentemente, permitindo que até 16 arquivos sejam processados simultaneamente. Este limite foi estabelecido para evitar sobrecarga do sistema e garantir um processamento eficiente dos arquivos CSV em paralelo.

## Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo LICENSE para detalhes. 