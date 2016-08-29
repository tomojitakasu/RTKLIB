object KeyDialog: TKeyDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Keyword Replacement in File Path'
  ClientHeight = 133
  ClientWidth = 316
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
    Left = 12
    Top = 12
    Width = 17
    Height = 13
    Caption = '%Y'
  end
  object Label2: TLabel
    Left = 12
    Top = 26
    Width = 17
    Height = 13
    Caption = '%y'
  end
  object Label3: TLabel
    Left = 12
    Top = 40
    Width = 19
    Height = 13
    Caption = '%m'
  end
  object Label4: TLabel
    Left = 12
    Top = 54
    Width = 17
    Height = 13
    Caption = '%d'
  end
  object Label5: TLabel
    Left = 12
    Top = 68
    Width = 17
    Height = 13
    Caption = '%n'
  end
  object Label6: TLabel
    Left = 12
    Top = 82
    Width = 21
    Height = 13
    Caption = '%W'
  end
  object Label7: TLabel
    Left = 12
    Top = 98
    Width = 18
    Height = 13
    Caption = '%D'
  end
  object Label8: TLabel
    Left = 156
    Top = 12
    Width = 17
    Height = 13
    Caption = '%h'
  end
  object Label9: TLabel
    Left = 156
    Top = 26
    Width = 19
    Height = 13
    Caption = '%M'
  end
  object Label10: TLabel
    Left = 156
    Top = 40
    Width = 17
    Height = 13
    Caption = '%S'
  end
  object Label11: TLabel
    Left = 156
    Top = 54
    Width = 18
    Height = 13
    Caption = '%H'
  end
  object Label12: TLabel
    Left = 36
    Top = 12
    Width = 64
    Height = 13
    Caption = ': Year (yyyy)'
  end
  object Label13: TLabel
    Left = 36
    Top = 26
    Width = 52
    Height = 13
    Caption = ': Year (yy)'
  end
  object Label14: TLabel
    Left = 36
    Top = 40
    Width = 64
    Height = 13
    Caption = ': Month (mm)'
  end
  object Label15: TLabel
    Left = 36
    Top = 54
    Width = 95
    Height = 13
    Caption = ': Day of Month (dd)'
  end
  object Label16: TLabel
    Left = 36
    Top = 68
    Width = 93
    Height = 13
    Caption = ': Day of Year (ddd)'
  end
  object Label17: TLabel
    Left = 36
    Top = 82
    Width = 115
    Height = 13
    Caption = ': GPS Week No (wwww)'
  end
  object Label18: TLabel
    Left = 36
    Top = 96
    Width = 96
    Height = 13
    Caption = ': Day of Week (0-6)'
  end
  object Label19: TLabel
    Left = 180
    Top = 12
    Width = 69
    Height = 13
    Caption = ': Hour (00-23)'
  end
  object Label20: TLabel
    Left = 180
    Top = 26
    Width = 78
    Height = 13
    Caption = ': Minute (00-59)'
  end
  object Label21: TLabel
    Left = 180
    Top = 40
    Width = 81
    Height = 13
    Caption = ': Second (00:59)'
  end
  object Label22: TLabel
    Left = 180
    Top = 54
    Width = 111
    Height = 13
    Caption = ': Hour Code (a,b,...,x)'
  end
  object Label23: TLabel
    Left = 156
    Top = 68
    Width = 23
    Height = 13
    Caption = '%ha'
  end
  object Label24: TLabel
    Left = 180
    Top = 68
    Width = 117
    Height = 13
    Caption = ': 3H Hour (00,03,...,21)'
  end
  object Label25: TLabel
    Left = 156
    Top = 82
    Width = 23
    Height = 13
    Caption = '%hb'
  end
  object Label26: TLabel
    Left = 180
    Top = 82
    Width = 117
    Height = 13
    Caption = ': 6H Hour (00,06,12,18)'
  end
  object Label27: TLabel
    Left = 156
    Top = 96
    Width = 22
    Height = 13
    Caption = '%hc'
  end
  object Label28: TLabel
    Left = 180
    Top = 96
    Width = 91
    Height = 13
    Caption = ': 12H Hour (00,12)'
  end
  object Label29: TLabel
    Left = 12
    Top = 112
    Width = 15
    Height = 13
    Caption = '%r'
  end
  object Label30: TLabel
    Left = 36
    Top = 110
    Width = 55
    Height = 13
    Caption = ': Station ID'
  end
  object Label31: TLabel
    Left = 156
    Top = 110
    Width = 17
    Height = 13
    Caption = '%b'
  end
  object Label32: TLabel
    Left = 180
    Top = 110
    Width = 81
    Height = 13
    Caption = ': Base Station ID'
  end
  object Label33: TLabel
    Left = 156
    Top = 68
    Width = 16
    Height = 13
    Caption = '%s'
  end
  object Label34: TLabel
    Left = 180
    Top = 68
    Width = 117
    Height = 13
    Caption = ': Station ID (lower case)'
  end
  object Label35: TLabel
    Left = 156
    Top = 82
    Width = 17
    Height = 13
    Caption = '%S'
  end
  object Label36: TLabel
    Left = 180
    Top = 82
    Width = 123
    Height = 13
    Caption = ': Station ID (UPPER case)'
  end
  object Label37: TLabel
    Left = 156
    Top = 40
    Width = 18
    Height = 13
    Caption = '%N'
  end
  object Label38: TLabel
    Left = 180
    Top = 40
    Width = 94
    Height = 13
    Caption = ': Sequence Number'
  end
  object BtnOk: TButton
    Left = 280
    Top = 100
    Width = 33
    Height = 27
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
end
