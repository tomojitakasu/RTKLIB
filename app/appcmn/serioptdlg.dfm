object SerialOptDialog: TSerialOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Serial Options'
  ClientHeight = 140
  ClientWidth = 310
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 36
    Width = 60
    Height = 13
    Caption = 'Bitrate (bps)'
  end
  object Label3: TLabel
    Left = 10
    Top = 12
    Width = 20
    Height = 13
    Caption = 'Port'
  end
  object Label2: TLabel
    Left = 10
    Top = 60
    Width = 44
    Height = 13
    Caption = 'Byte Size'
  end
  object Label4: TLabel
    Left = 162
    Top = 12
    Width = 28
    Height = 13
    Caption = 'Parity'
  end
  object Label5: TLabel
    Left = 162
    Top = 36
    Width = 42
    Height = 13
    Caption = 'Stop Bits'
  end
  object Label8: TLabel
    Left = 162
    Top = 60
    Width = 60
    Height = 13
    Caption = 'Flow Control'
  end
  object BtnOk: TButton
    Left = 123
    Top = 108
    Width = 89
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 215
    Top = 108
    Width = 89
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BitRate: TComboBox
    Left = 74
    Top = 32
    Width = 77
    Height = 21
    DropDownCount = 12
    ItemIndex = 5
    TabOrder = 3
    Text = '9600'
    Items.Strings = (
      '300'
      '600'
      '1200'
      '2400'
      '4800'
      '9600'
      '19200'
      '38400'
      '57600'
      '115200'
      '230400')
  end
  object Port: TComboBox
    Left = 74
    Top = 8
    Width = 77
    Height = 21
    DropDownCount = 10
    ItemIndex = 0
    TabOrder = 2
    Text = 'COM1'
    Items.Strings = (
      'COM1')
  end
  object ByteSize: TComboBox
    Left = 74
    Top = 56
    Width = 77
    Height = 21
    Style = csDropDownList
    ItemIndex = 1
    TabOrder = 4
    Text = '8 bits'
    Items.Strings = (
      '7 bits'
      '8 bits')
  end
  object Parity: TComboBox
    Left = 226
    Top = 8
    Width = 77
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 5
    Text = 'None'
    Items.Strings = (
      'None'
      'Even'
      'Odd')
  end
  object StopBits: TComboBox
    Left = 226
    Top = 32
    Width = 77
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 6
    Text = '1 bit'
    Items.Strings = (
      '1 bit'
      '2 bits')
  end
  object FlowCtr: TComboBox
    Left = 226
    Top = 56
    Width = 77
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 7
    Text = 'None'
    Items.Strings = (
      'None'
      'RTS/CTS')
  end
  object BtnCmd: TButton
    Left = 9
    Top = 108
    Width = 89
    Height = 29
    Caption = '&Commands...'
    TabOrder = 8
    Visible = False
    OnClick = BtnCmdClick
  end
  object OutTcpPort: TCheckBox
    Left = 10
    Top = 84
    Width = 195
    Height = 17
    Caption = 'Output Received Stream to  TCP Port :'
    TabOrder = 9
    OnClick = OutTcpPortClick
  end
  object TcpPort: TEdit
    Left = 225
    Top = 82
    Width = 78
    Height = 21
    TabOrder = 10
  end
end
