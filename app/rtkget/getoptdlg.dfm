object DownOptDialog: TDownOptDialog
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 257
  ClientWidth = 356
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
  object BtnOk: TButton
    Left = 193
    Top = 227
    Width = 79
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 273
    Top = 227
    Width = 79
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 356
    Height = 225
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object Label1: TLabel
      Left = 14
      Top = 188
      Width = 129
      Height = 13
      Caption = 'URL List File for GNSS Data'
    end
    object Label2: TLabel
      Left = 14
      Top = 100
      Width = 98
      Height = 13
      Caption = 'Output Debug Trace'
    end
    object Label3: TLabel
      Left = 14
      Top = 122
      Width = 70
      Height = 13
      Caption = 'Proxy Address'
    end
    object Label4: TLabel
      Left = 14
      Top = 145
      Width = 86
      Height = 13
      Caption = 'Download Log File'
    end
    object Label5: TLabel
      Left = 14
      Top = 78
      Width = 101
      Height = 13
      Caption = 'Date Format for Test'
    end
    object Label6: TLabel
      Left = 14
      Top = 56
      Width = 92
      Height = 13
      Caption = '# Columns for Test'
    end
    object Proxy: TEdit
      Left = 120
      Top = 119
      Width = 231
      Height = 21
      Enabled = False
      TabOrder = 4
    end
    object UrlFile: TEdit
      Left = 6
      Top = 203
      Width = 345
      Height = 21
      TabOrder = 8
    end
    object BtnUrlFile: TButton
      Left = 332
      Top = 184
      Width = 19
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 9
      OnClick = BtnUrlFileClick
    end
    object TraceLevel: TComboBox
      Left = 120
      Top = 97
      Width = 84
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 3
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Level 1'
        'Level 2'
        'Level 3'
        'Level 4'
        'Level 5')
    end
    object LogFile: TEdit
      Left = 6
      Top = 161
      Width = 345
      Height = 21
      TabOrder = 6
    end
    object BtnLogFile: TButton
      Left = 332
      Top = 142
      Width = 19
      Height = 19
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 7
      OnClick = BtnLogFileClick
    end
    object LogAppend: TCheckBox
      Left = 120
      Top = 143
      Width = 97
      Height = 17
      Caption = 'Append'
      TabOrder = 5
    end
    object HoldErr: TCheckBox
      Left = 13
      Top = 11
      Width = 241
      Height = 17
      Caption = ' Keep Error Info on Download Error (*.err)'
      TabOrder = 0
    end
    object HoldList: TCheckBox
      Left = 13
      Top = 32
      Width = 244
      Height = 17
      Caption = ' Keep Remote Directory Listings (.listing)'
      TabOrder = 1
    end
    object DateFormat: TComboBox
      Left = 120
      Top = 75
      Width = 84
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 2
      Text = 'YYYY-DOY'
      Items.Strings = (
        'YYYY-DOY'
        'YYYY/mm/dd'
        'Week')
    end
    object NCol: TEdit
      Left = 120
      Top = 53
      Width = 85
      Height = 21
      TabOrder = 10
      Text = '35'
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'Text File (*.txt)|*.txt|All File (*.*)|*.*'
    Left = 19
    Top = 227
  end
  object SaveDialog: TSaveDialog
    Filter = 
      'Log File (*.log)|*.log|Text File (*.txt)|*.txt|All File (*.*)|*.' +
      '*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 47
    Top = 227
  end
end
