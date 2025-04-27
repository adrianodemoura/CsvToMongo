# CSV para MongoDB

Este projeto é um script em C para importar arquivos CSV para o MongoDB, com suporte a múltiplas threads e controle de recursos.

## Características

- Importação de múltiplos arquivos CSV em paralelo
- Controle de uso de memória RAM (configurável)
- Controle de número de threads (configurável)
- Estrutura de dados otimizada para MongoDB
- Sistema de logs detalhado
- Configuração flexível via arquivo JSON
- Suporte a campos aninhados (emails e telefones como subcoleções)
- Validação de campos conforme arquivo fields.txt

## Requisitos

- GCC 7.0 ou superior
- libmongoc 1.0 ou superior
- libbson 1.0 ou superior
- libjson-c
- libcsv
- pkg-config

## Instalação

1. Instale as dependências:

```bash
sudo apt-get update
sudo apt-get install -y \
    libmongoc-dev \
    libbson-dev \
    libjson-c-dev \
    libcsv-dev \
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
    "mongodb_database": "telefonesonline",
    "mongodb_collection": "pessoas",
    "mongodb_username": "root",
    "mongodb_password": "Mongo6701!",
    "max_threads": 8,
    "memory_limit_percent": 70
}
```

### Campos de Configuração

- `mongodb_host`: Host do MongoDB
- `mongodb_port`: Porta do MongoDB
- `mongodb_database`: Nome do banco de dados
- `mongodb_collection`: Nome da coleção
- `mongodb_username`: Usuário do MongoDB
- `mongodb_password`: Senha do MongoDB
- `max_threads`: Número máximo de threads para processamento
- `memory_limit_percent`: Limite de uso de memória RAM (em porcentagem)

## Uso

1. Coloque seus arquivos CSV no diretório `files_csv/`
2. Certifique-se que o arquivo `fields.txt` na raiz do projeto contém a lista de campos válidos
3. Execute o script:

```bash
./bin/csv_to_mongo
```

## Estrutura do Projeto

```
.
├── config/
│   ├── config.json        # Configurações do projeto
│   ├── config_loader.c    # Carregador de configurações
│   └── config_loader.h    # Header do carregador
├── data/
│   ├── person.c          # Implementação da estrutura de pessoa
│   └── person.h          # Header da estrutura
├── mongodb/
│   ├── mongodb_client.c  # Cliente MongoDB
│   └── mongodb_client.h  # Header do cliente
├── csv/
│   ├── csv_parser.c      # Parser CSV
│   └── csv_parser.h      # Header do parser
├── utils/
│   ├── memory_manager.c  # Gerenciador de memória
│   ├── memory_manager.h  # Header do gerenciador
│   ├── logger.c          # Sistema de logs
│   └── logger.h          # Header do logger
├── src/
│   └── main.c           # Ponto de entrada do programa
├── files_csv/           # Diretório para arquivos CSV
├── fields.txt           # Lista de campos válidos
├── Makefile            # Script de compilação
└── README.md           # Este arquivo
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
    "data_atualizacao": "2024-03-20",
    "nota": "5",
    "banco": "001",
    "cpf_conjuge": "987.654.321-00",
    "serv_publico": "N",
    "data_obito": null,
    "cidade": "São Paulo",
    "endereco": "Rua Exemplo, 123",
    "bairro": "Centro",
    "cep": "01234-567",
    "uf": "SP",
    "contacts": {
        "phones": [
            "11999999999",
            "11988888888"
        ],
        "emails": [
            "joao@email.com",
            "joao.silva@email.com"
        ]
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

- O script só importa os campos listados no arquivo `fields.txt`
- Campos de email e telefone são agrupados em uma subcoleção `contacts`
- O número máximo de threads é limitado pela configuração
- O uso de memória é limitado pela configuração

## Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo LICENSE para detalhes. 