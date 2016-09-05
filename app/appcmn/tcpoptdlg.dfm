object TcpOptDialog: TTcpOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'TCP Options'
  ClientHeight = 153
  ClientWidth = 318
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
  object LabelAddr: TLabel
    Left = 8
    Top = 4
    Width = 96
    Height = 13
    Caption = 'TCP Server Address'
  end
  object LabelPort: TLabel
    Left = 212
    Top = 4
    Width = 20
    Height = 13
    Caption = 'Port'
  end
  object LabelUser: TLabel
    Left = 110
    Top = 42
    Width = 37
    Height = 13
    Caption = 'User-ID'
  end
  object LabelPasswd: TLabel
    Left = 212
    Top = 42
    Width = 46
    Height = 13
    Caption = 'Password'
  end
  object LabelMntPnt: TLabel
    Left = 8
    Top = 42
    Width = 54
    Height = 13
    Caption = 'Mountpoint'
  end
  object LabelStr: TLabel
    Left = 8
    Top = 82
    Width = 28
    Height = 13
    Caption = 'String'
  end
  object BtnCancel: TButton
    Left = 223
    Top = 121
    Width = 89
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 130
    Top = 121
    Width = 89
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Port: TEdit
    Left = 210
    Top = 18
    Width = 101
    Height = 21
    TabOrder = 4
  end
  object User: TEdit
    Left = 108
    Top = 56
    Width = 101
    Height = 21
    TabOrder = 6
  end
  object Passwd: TEdit
    Left = 210
    Top = 56
    Width = 101
    Height = 21
    PasswordChar = '*'
    TabOrder = 7
  end
  object Str: TEdit
    Left = 6
    Top = 96
    Width = 305
    Height = 21
    TabOrder = 8
  end
  object Addr: TComboBox
    Left = 6
    Top = 18
    Width = 203
    Height = 21
    AutoComplete = False
    DropDownCount = 16
    TabOrder = 3
  end
  object MntPnt: TComboBox
    Left = 6
    Top = 56
    Width = 101
    Height = 21
    AutoComplete = False
    DropDownCount = 16
    TabOrder = 5
  end
  object BtnNtrip: TButton
    Left = 6
    Top = 121
    Width = 89
    Height = 29
    Caption = '&Ntrip...'
    TabOrder = 2
    OnClick = BtnNtripClick
  end
end
