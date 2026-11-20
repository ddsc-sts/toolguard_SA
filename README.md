# TOOLGUARD

### Sistema de monitoramento didático de vibração e desgaste

**Integrante:** Daniel dos Santos Costa  
**Turma:** DS-V1  
**Unidade curricular:** Internet das Coisas  
**Docente:** Sergio Luiz da Silveira

---

Protótipo didático para a avaliação prática de IoT. O Arduino calcula um indicador RMS de vibração, compara o resultado com uma referência de calibração e apresenta a condição estimada no LCD e no LED RGB.

## Objetivo

Monitorar alterações em um sinal de vibração e apresentar uma classificação simples da condição de uma ferramenta ou conjunto rotativo. Na versão atual, o sinal de vibração é gerado matematicamente pelo programa para demonstrar o cálculo e as mudanças de estado no simulador.

## Estados de operação

| Estado | Razão entre RMS atual e referência | Indicador visual |
|---|---:|---|
| **NORMAL** | até 1,20 | Verde |
| **ATENÇÃO** | acima de 1,20 e até 2,00 | Laranja |
| **CRÍTICO** | acima de 2,00 | Vermelho |

Os limites são parâmetros didáticos da demonstração e não foram validados para manutenção industrial.

## Componentes

| Componente | Quantidade | Função e ligação principal |
|---|---:|---|
| Arduino Uno R3 | 1 | Controlador; alimentação e terra comuns. |
| LCD 16×2 I²C PCF8574 | 1 | Exibe RMS, variação e estado; endereço `0x27`, conectado ao barramento I²C. |
| Potenciômetro 250 kΩ | 1 | Cursor em A0; ajusta o fator simulado e o PWM do motor. |
| Motor DC | 1 | Atuador do protótipo, acionado pelo MOSFET no circuito de potência. |
| MOSFET canal N | 1 | Chaveia o motor; sinal de controle no D10. |
| LED RGB | 1 | Indica os três estados; canais nos pinos D3, D5 e D6. |
| Resistores de 330 Ω | 3 | Um em série com cada canal do LED RGB. |
| Resistor de 1 kΩ | 1 | Listado no BOM para o circuito de acionamento do motor. |
| Breadboard pequena | 1 | Base para montagem e interligação dos componentes. |

Consulte `hardware/bom.csv` e os diagramas em `hardware/`. Os esquemas recebidos mostram variantes do circuito; confira qual corresponde à montagem apresentada.

## Como funciona

1. Ao iniciar, o Arduino apresenta a tela de abertura e calibra uma referência RMS.
2. O programa gera um sinal senoidal simulado de 50 Hz, com amplitude alterada pelo potenciômetro.
3. O Arduino coleta janelas de amostras, calcula o RMS e compara o valor com a referência.
4. O LCD mostra RMS, variação percentual e estado; o LED RGB indica o estado pela cor.
5. O potenciômetro também controla diretamente a velocidade do motor por PWM no pino D10, através do MOSFET.
6. A telemetria pode ser acompanhada no monitor serial a 9600 baud.

## Simulação

A simulação compartilhada está registrada em [`hardware/tinkercad_link.txt`](hardware/tinkercad_link.txt). Inicie a simulação, aguarde a calibração e varie o potenciômetro para observar os estados NORMAL, ATENÇÃO e CRÍTICO no LCD, no LED e no monitor serial.

## Arquivos do repositório

- [Documentação técnica em PDF](docs/Documentacao_Tecnica_TOOLGUARD.pdf)
- [Documentação editável em DOCX](docs/Documentacao_Tecnica_TOOLGUARD.docx)
- [Sketch Arduino](src/toolguard_tinkercad.ino)
- [Lista de materiais](hardware/bom.csv)
- Diagramas e link do Tinkercad em `hardware/`

## Escopo futuro

A instalação de um sensor piezoelétrico e a leitura de vibração real são possibilidades para uma versão futura. O piezo **não faz parte do projeto atual**: não está conectado na montagem, não aparece no BOM e não é usado pelo sketch entregue. Uma implementação futura precisará definir o condicionamento do sinal, a fixação mecânica e os limites de leitura adequados.

## Limitações da versão atual

- O sinal de vibração é simulado, não medido por um sensor físico.
- O motor recebe o PWM do potenciômetro. O PWM associado ao estado é calculado e exibido na serial, mas ainda não é aplicado para limitar ou interromper o motor.
- Os limiares RMS são demonstrativos e precisam de validação em condições controladas antes de qualquer uso prático.

## Versionamento

O histórico Git registra as alterações do projeto e da documentação. Use mensagens de commit que descrevam cada mudança. Após publicar este repositório no GitHub, copie o link para o AVA junto com a documentação e as evidências da simulação.
