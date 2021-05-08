object TcpOptDialog: TTcpOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'TCP Options'
  ClientHeight = 124
  ClientWidth = 365
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
    Top = 3
    Width = 96
    Height = 13
    Caption = 'TCP Server Address'
  end
  object LabelPort: TLabel
    Left = 258
    Top = 3
    Width = 20
    Height = 13
    Caption = 'Port'
  end
  object LabelUser: TLabel
    Left = 126
    Top = 42
    Width = 36
    Height = 13
    Caption = 'User ID'
  end
  object LabelPasswd: TLabel
    Left = 244
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
  object BtnMountp: TButton
    Left = 6
    Top = 89
    Width = 100
    Height = 29
    Caption = 'Mountp Options...'
    TabOrder = 9
    Visible = False
    OnClick = BtnMountpClick
  end
  object BtnCancel: TButton
    Left = 273
    Top = 89
    Width = 84
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 187
    Top = 89
    Width = 84
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object Port: TEdit
    Left = 255
    Top = 17
    Width = 101
    Height = 21
    TabOrder = 3
  end
  object User: TEdit
    Left = 124
    Top = 56
    Width = 116
    Height = 21
    TabOrder = 5
  end
  object Passwd: TEdit
    Left = 242
    Top = 56
    Width = 114
    Height = 21
    PasswordChar = '*'
    TabOrder = 6
  end
  object Addr: TComboBox
    Left = 6
    Top = 17
    Width = 247
    Height = 21
    AutoComplete = False
    DropDownCount = 16
    TabOrder = 2
  end
  object MntPnt: TComboBox
    Left = 6
    Top = 56
    Width = 116
    Height = 21
    AutoComplete = False
    DropDownCount = 16
    TabOrder = 4
  end
  object BtnNtrip: TButton
    Left = 92
    Top = 89
    Width = 84
    Height = 29
    Caption = '&Get Mountp'
    TabOrder = 8
    Visible = False
    OnClick = BtnNtripClick
  end
  object BtnBrows: TButton
    Left = 6
    Top = 89
    Width = 84
    Height = 29
    Caption = '&Browse...'
    TabOrder = 7
    Visible = False
    OnClick = BtnBrowsClick
  end
end
