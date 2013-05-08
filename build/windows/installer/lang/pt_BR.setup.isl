[Messages]
;InfoBefore page is used instead of license page because the GPL only concerns distribution, not use, and as such doesn't have to be accepted
WizardInfoBefore=Acordo de Licenciamento
AboutSetupNote=Instalador feito por Jernej Simon�i�, jernej-picman@ena.si%n%nImagem na p�gina de abertura do instalador po Alexia_Death%nImagem no final do instalador por Jakub Steiner
WinVersionTooLowError= Esta vers�o do PICMAN requer o Windows XP com o Service Pack 3 ou uma vers�o mais recente do Windows.

[CustomMessages]
;shown before the wizard starts on development versions of PICMAN
DevelopmentWarningTitle=Vers�o de Desenvolvimento
DevelopmentWarning=Esta � uma vers�o de desenvolvimento do PICMAN. Sendo assim, algumas funcionalidades n�o est�o prontas, e ele pode ser inst�vel. Se voc� encontrar algum problema, primeiro verifique se ele j� n�o foi arrumado no GIT antes de contatar os desenvolvedores.%nEsta vers�o do PICMAN n�o foi feita para ser usada no dia a dia, uma vez que ela pode ser inst�vel e pode por o seu trabalho em risco. Voc� quer continuar com esta instala��o mesmo assim?
;DevelopmentWarning=Esta � uma vers�o de desenvolvimento do instalador do PICMAN. Ela n�o foi testada tanto quanto o instalador est�vel, o que pode resultar no PICMAN n�o ser corretamente instalado. Por favor, reporte quaisquer problemas que voc� encontrar no bugizlla do PICMAN (Installer component):%n_https://bugzilla.gnome.org/enter_bug.cgi?product=PICMAN%n%nH� alguns problemas conhecidos neste instalador:%n- Arquivos TIFF n�o est�o abrindo%n-Tamanho dos arquivos n�o est�o sendo exibidos corretamente%nPor favor n�o reporte esses problemas, uma vez que j� estamos a par dos mesmos.%n%nVoc� quer continuar com a instala��o mesmo assim?
DevelopmentButtonContinue=&Continuar
DevelopmentButtonExit=Sair

;XPSP3Recommended=Aviso: Voc� est� executando uma vers�o n�o suportada do Windows. Por favor atualize pelo menos para WIndows XP com Service Pack 3 antes de mencionar quaisquer problemas.
SSERequired=Esta vers�o do PICMAN requer um processador que suporte as instru��es SSE.

Require32BPPTitle=Problema nas configura��es de tela
Require32BPP=O instalador detectou que seu windows n�o est� rodando num modo de tela de 32 bits por pixel. Isso sabidamente pode causar alguns problemas de estabilidade com o PICMAN, ent�o � recomendado mudar a profundidade de cores da tela para 32BPP antesd e continuar.
Require32BPPContinue=&Continuar
Require32BPPExit=Sair

InstallOrCustomize=O PICMAN est� pronto para ser instalado. Clique no bot�o Instalar para instalar com as configura��es padr�o, ou clique no bot�o Personalizar se voc� deseja ter mais controle sobre o que ser� instalado.
Install=&Instalar
Customize=&Personalizar

;setup types
TypeCompact=Instala��o Compacta
TypeCustom=Instala��o personalizada
TypeFull=Instala��o completa

;text above component description
ComponentsDescription=Descri��o
;components
ComponentsPicman=PICMAN
ComponentsPicmanDescription=PICMAN e todos os plug-ins padr�o
ComponentsDeps=Bibliotecas de execu��o
ComponentsDepsDescription=Biblitoecas de execu��o utilizadas pelo PICMAN, incluindo o ambiente de execu��o do GTK+
ComponentsGtkWimp=Motor do GTK+ para MS-Windows
ComponentsGtkWimpDescription=Apar�ncia nativa de Windows para o PICMAN
ComponentsCompat=Suporte para plug-ins antigos
ComponentsCompatDescription=Instalar bibliotecas necess�rias para plug-ins antigos de terceiros
ComponentsTranslations=Tradu��es
ComponentsTranslationsDescription=Tradu��es
ComponentsPython=Suporte a scripts em Python
ComponentsPythonDescription=Permite que voc� use plug-ins escritos na linguagem Python(necess�rio para algumas funcionalidades).
ComponentsGhostscript=Suporte a Postscript
ComponentsGhostscriptDescription=Permite que o PICMAN possa abrir arquivos Postscript
;only when installing on x64 Windows
ComponentsPicman32=Suporte a plug-ins de 32-bit
ComponentsPicman32Description=Inclui arquivos necess�rios para o uso de plug-ins de 32bits.%nNecess�rio para o suporte a Python.

;additional installation tasks
AdditionalIcons=Icones adicionais:
AdditionalIconsDesktop=Criar um �cone de na �rea de Trabalho
AdditionalIconsQuickLaunch=Criar um �cone de Lan�amento R�pido

RemoveOldPICMAN=Remover a vers�o anterior do PICMAN

;%1 is replaced by file name; these messages should never appear (unless user runs out of disk space at the exact right moment)
ErrorChangingEnviron=Houve um problema ao atualizar o ambiente em %1. Se voc� ver algum erro ao carregar os plug-ins, tente desisntalar e re-instalar o PICMAN
ErrorExtractingTemp=Erro ao extrair dados tempor�rios.
ErrorUpdatingPython=Erro ao atualizar informa��es do interpretador Python.
ErrorReadingPicmanRC=Houve um erro ao atualizar %1.
ErrorUpdatingPicmanRC=Houve um erro ao atualizar o arquivo de configura��o do PICMAN %1.

;displayed in Explorer's right-click menu
OpenWithPicman=Editar com o PICMAN

;file associations page
SelectAssociationsCaption=Escolha as associa��es de arquivos
SelectAssociationsExtensions=Exten��es:
SelectAssociationsInfo1=Selecione os tipos de arquivos que voc� deseja associar com  PICMAN
SelectAssociationsInfo2=Isso far� com que os arquivos selecionados abram no PICMAN quando voc� clicar nos mesos no Explorer.
SelectAssociationsSelectAll=Selecionar &todos
SelectAssociationsUnselectAll=&Des-selecionar todos
SelectAssociationsSelectUnused=Selecionar os &n�o utilizados

;shown on summary screen just before starting the install
ReadyMemoAssociations=Tipos de arquivo que ser�o associados ao PICMAN:

RemovingOldVersion=Removendo vers�o anterior do PICMAN:
;%1 = version, %2 = installation directory
;ran uninstaller, but it returned an error, or didn't remove everything
RemovingOldVersionFailed=PICMAN %1 n�o pode ser instalado por cima da sua vers�o do PICMAN instalada atualmente, e a desinstala��o automatica da vers�o antiga falhou.%n%nPor favor, remova manualmente a vers�o anterior do PICMAN antes de instalar esta vers�o em %2, ou escolha a Instala��o Personalizada e selecione uma pasta diferente para instala��o.%n%nA instala��o ser� encerrada.
;couldn't find an uninstaller, or found several uninstallers
RemovingOldVersionCantUninstall=PICMAN %1 n�o pode ser instalado por cima da sua vers�o do PICMAN instalada atualmente, e  Instalador n�o pode determinar como remover a vers�o anterior automaticamente.%n%nPor favor, remova manualmente a vers�o anterior do PICMAN antes de instalar esta vers�o em %2, ou escolha a Instala��o Personalizada e selecione uma pasta diferente para instala��o.%n%nA instala��o ser� encerrada.

RebootRequiredFirst=A vers�o anterior do PICMAN foi removida com sucesso, mas o Windows deve ser reiniciado antes que o instalador possa continuar.%n%nAp�s reiniciar seu computador, o Instalador vai continuar assim que um Administrador fizer o login.

;displayed if restart settings couldn't be read, or if the setup couldn't re-run itself
ErrorRestartingSetup=Houve um erro ao reiniciar o instalador do PICMAN. (%1)

;displayed while the files are being extracted; note the capitalisation!
Billboard1=Lembre-se: o PICMAN � Software Livre.%n%nPor favor visite
;www.picman.org (displayed between Billboard1 and Billboard2)
Billboard2=para atualiza��es gratuitas

SettingUpAssociations=Configurando associa��es de arquivo...
SettingUpPyPicman=Configurando ambiente para a extens�o Python do PICMAN...
SettingUpEnvironment=Configurando ambiente do PICMAN...
SettingUpPicmanRC=Configurando o suporte a plug-ins de 32bit...

;displayed on last page
LaunchPicman=Iniciar o PICMAN

;shown during uninstall when removing add-ons
UninstallingAddOnCaption=Removendo o componente extra

InternalError=Erro interno (%1).

;used by installer for add-ons (currently only help)
DirNotPicman=PICMAN n�o parece estar instalado no diret�rio selecionado. Continuar mesmo assim?
