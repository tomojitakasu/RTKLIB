object MntpOptDialog: TMntpOptDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Mountpoint Options'
  ClientHeight = 235
  ClientWidth = 360
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
  object Label14: TLabel
    Left = 107
    Top = 4
    Width = 47
    Height = 13
    Caption = 'Source ID'
  end
  object Label15: TLabel
    Left = 9
    Top = 80
    Width = 33
    Height = 13
    Caption = 'Carrier'
  end
  object Label16: TLabel
    Left = 9
    Top = 156
    Width = 49
    Height = 13
    Caption = 'Generator'
  end
  object Label1: TLabel
    Left = 10
    Top = 42
    Width = 69
    Height = 13
    Caption = 'Format Details'
  end
  object Label2: TLabel
    Left = 266
    Top = 4
    Width = 34
    Height = 13
    Caption = 'Format'
  end
  object Label3: TLabel
    Left = 87
    Top = 118
    Width = 39
    Height = 13
    Caption = 'Country'
  end
  object Label4: TLabel
    Left = 9
    Top = 118
    Width = 40
    Height = 13
    Caption = 'Network'
  end
  object Label5: TLabel
    Left = 271
    Top = 118
    Width = 28
    Height = 13
    Caption = 'NMEA'
  end
  object Label6: TLabel
    Left = 313
    Top = 118
    Width = 36
    Height = 13
    Caption = 'Soution'
  end
  object Label7: TLabel
    Left = 52
    Top = 80
    Width = 61
    Height = 13
    Caption = 'Nav-System '
  end
  object Label8: TLabel
    Left = 131
    Top = 118
    Width = 39
    Height = 13
    Caption = 'Latitude'
  end
  object Label9: TLabel
    Left = 201
    Top = 118
    Width = 47
    Height = 13
    Caption = 'Longitude'
  end
  object Label10: TLabel
    Left = 199
    Top = 156
    Width = 68
    Height = 13
    Caption = 'Compr-encryp'
  end
  object Label11: TLabel
    Left = 271
    Top = 156
    Width = 23
    Height = 13
    Caption = 'Auth'
  end
  object Label12: TLabel
    Left = 313
    Top = 156
    Width = 18
    Height = 13
    Caption = 'Fee'
  end
  object Label13: TLabel
    Left = 9
    Top = 192
    Width = 32
    Height = 13
    Caption = 'Bitrate'
  end
  object Label17: TLabel
    Left = 9
    Top = 4
    Width = 54
    Height = 13
    Caption = 'Mountpoint'
  end
  object BtnCancel: TButton
    Left = 263
    Top = 201
    Width = 89
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object BtnOk: TButton
    Left = 171
    Top = 201
    Width = 89
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object SrcTbl1: TEdit
    Left = 106
    Top = 18
    Width = 156
    Height = 21
    TabOrder = 3
  end
  object SrcTbl2: TEdit
    Left = 265
    Top = 18
    Width = 86
    Height = 21
    TabOrder = 4
  end
  object SrcTbl3: TEdit
    Left = 8
    Top = 56
    Width = 343
    Height = 21
    TabOrder = 5
  end
  object SrcTbl5: TEdit
    Left = 50
    Top = 94
    Width = 301
    Height = 21
    TabOrder = 7
  end
  object SrcTbl6: TEdit
    Left = 8
    Top = 132
    Width = 75
    Height = 21
    TabOrder = 8
  end
  object SrcTbl7: TEdit
    Left = 86
    Top = 132
    Width = 41
    Height = 21
    TabOrder = 9
  end
  object SrcTbl8: TEdit
    Left = 130
    Top = 132
    Width = 67
    Height = 21
    TabOrder = 10
  end
  object SrcTbl9: TEdit
    Left = 200
    Top = 132
    Width = 67
    Height = 21
    TabOrder = 11
  end
  object SrcTbl12: TEdit
    Left = 8
    Top = 170
    Width = 189
    Height = 21
    TabOrder = 14
  end
  object SrcTbl4: TComboBox
    Left = 8
    Top = 94
    Width = 39
    Height = 21
    ItemIndex = 0
    TabOrder = 6
    Text = '0'
    Items.Strings = (
      '0'
      '1'
      '2'
      '3')
  end
  object SrcTbl10: TComboBox
    Left = 270
    Top = 132
    Width = 39
    Height = 21
    ItemIndex = 0
    TabOrder = 12
    Text = '0'
    Items.Strings = (
      '0'
      '1')
  end
  object SrcTbl11: TComboBox
    Left = 312
    Top = 132
    Width = 39
    Height = 21
    ItemIndex = 0
    TabOrder = 13
    Text = '0'
    Items.Strings = (
      '0'
      '1')
  end
  object SrcTbl13: TEdit
    Left = 200
    Top = 170
    Width = 67
    Height = 21
    TabOrder = 15
  end
  object SrcTbl14: TComboBox
    Left = 270
    Top = 170
    Width = 39
    Height = 21
    ItemIndex = 0
    TabOrder = 16
    Text = 'N'
    Items.Strings = (
      'N'
      'B'
      'D')
  end
  object SrcTbl15: TComboBox
    Left = 312
    Top = 170
    Width = 39
    Height = 21
    ItemIndex = 0
    TabOrder = 17
    Text = 'N'
    Items.Strings = (
      'N'
      'Y')
  end
  object SrcTbl16: TEdit
    Left = 8
    Top = 206
    Width = 76
    Height = 21
    TabOrder = 18
  end
  object MntPntE: TEdit
    Left = 8
    Top = 18
    Width = 95
    Height = 21
    TabOrder = 2
  end
end
