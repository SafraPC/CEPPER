# CEPPER

CEPPER é um projeto interdisciplinar desenvolvido para a faculdade superior de tecnologia IFSP, onde devemos criar um algoritmo em C e seu devido fluxograma.

# O que é o CEPPER?

CEPPER é um projeto que irá capturar o CEP inserido pelo usuário e devolver suas específicações geográficas, como : Rua, Bairro, Logradouro, IBGE, etc...

#INSTRUÇÕES DE INSTALAÇÃO

---

#Iniciando Server Javascript

Para inicializar o CEP em sua máquina, será necessário: Node (Recomendada versão 14^), GCC (Compilador C), MySQL Server, MySQL Connector, Code-blocks, alguma IDE para criar o banco (DBeaver ou Workbench), Docker e Docker-compose.

Caso seja sua primeira vez inicializando o projeto, será necessário baixar todas as bibliotecas javascript utilizadas no projeto. Portanto, insira o seguinte comando no terminal, na raíz do projeto :

npm i

Em seguida, inicialize o servidor javascript

npm start

Caso tenha dado tudo certo, nenhuma mensagem de erro apareceu e houve uma mensagem de sucesso indicando que o servidor foi inicializado na porta 4000.

---

#Criando Script no Banco

Para que o programe rode perfeitamente, precisamos criar e estabilizar o banco, portanto, inicialize seu banco MySQL na porta 3306 e conecte a IDE de banco que desejar na devida porta.

Depois de ter conectado sua IDE ao banco, insira o seguinte script .sql :

create database cepper;

use cepper

create table users(
id smallint auto_increment primary key,
name varchar(75) not null,
born varchar(20) not null,
cpf varchar(20) not null,
email varchar(100) not null,
pass varchar(100) not null
);
create table ceps(
id smallint auto_increment primary key,
cep varchar(40),
logradouro varchar(40),
bairro varchar(40),
localidade varchar(40),
uf varchar(40),
ibge varchar(40),
gia varchar(40),
ddd varchar(40),
siafi varchar(40)
);

create table tbquery(
id smallint auto_increment primary key,
query varchar(350)
);

create table tberror(
id smallint auto_increment primary key,
error varchar(40)
);

---

#Inicializando programa C

Aqui é onde roda nosso programa principal, portanto, abra seu code-blocks na pasta C, localizada na raíz do projeto e execute o arquivo cepper.cbp, o mesmo inicializará o projeto para que possa ser executado.

Como é necessário utilizar MySQL no projeto, o mesmo é uma biblioteca externa, portanto injete o a lib em sua IDE para que o projeto seja rodado.

Caso seu SOP seja windows, baixe ConnectorC direto do site MySQL,

Linux: caso sua distribuição for ubuntu, baixe .rem do site MySQL ou instale via sudo apt-get install

CASO SUA DISTRIBUIÇÃO FOR ARCH, A UNICA FORMA É BAIXANDO AS LIBS MYSQL DA COMUNIDADE AUR.

Feito tudo, basta inicializar o projeto no codeblocks (arquivo main.c)

# OBRIGADO!
