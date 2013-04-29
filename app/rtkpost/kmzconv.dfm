object ConvDialog: TConvDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Google Earth Converter'
  ClientHeight = 261
  ClientWidth = 372
  Color = clBtnFace
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
  object BtnClose: TButton
    Left = 286
    Top = 239
    Width = 79
    Height = 21
    Caption = '&Close'
    TabOrder = 1
    OnClick = BtnCloseClick
  end
  object BtnConvert: TButton
    Left = 204
    Top = 239
    Width = 79
    Height = 21
    Caption = 'Con&vert'
    TabOrder = 0
    OnClick = BtnConvertClick
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 372
    Height = 238
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object Label1: TLabel
      Left = 12
      Top = 59
      Width = 54
      Height = 13
      Caption = 'Track Color'
    end
    object Label2: TLabel
      Left = 12
      Top = 82
      Width = 57
      Height = 13
      Caption = 'Points Color'
    end
    object Label3: TLabel
      Left = 198
      Top = 59
      Width = 74
      Height = 13
      Caption = 'Output Altitude'
    end
    object Label4: TLabel
      Left = 198
      Top = 82
      Width = 59
      Height = 13
      Caption = 'Output Time'
    end
    object Label5: TLabel
      Left = 198
      Top = 36
      Width = 37
      Height = 13
      Caption = 'Q-Flags'
      FocusControl = AddOffset
    end
    object Label7: TLabel
      Left = 12
      Top = 130
      Width = 121
      Height = 13
      Caption = 'Input/Output/GE Exe File'
    end
    object TimeIntF: TCheckBox
      Left = 8
      Top = 34
      Width = 83
      Height = 17
      Caption = 'Interval (s)'
      TabOrder = 9
      OnClick = TimeIntFClick
    end
    object TrackColor: TComboBox
      Left = 98
      Top = 55
      Width = 81
      Height = 21
      Style = csDropDownList
      ItemIndex = 5
      TabOrder = 11
      Text = 'Yellow'
      Items.Strings = (
        'OFF'
        'White'
        'Green'
        'Orange'
        'Red'
        'Yellow')
    end
    object PointColor: TComboBox
      Left = 98
      Top = 78
      Width = 81
      Height = 21
      Style = csDropDownList
      ItemIndex = 5
      TabOrder = 12
      Text = 'By Q-Flag'
      Items.Strings = (
        'OFF'
        'White'
        'Green'
        'Orange'
        'Red'
        'By Q-Flag')
    end
    object OutputAlt: TComboBox
      Left = 284
      Top = 55
      Width = 81
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 14
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Ellipsoidal'
        'Geodetic')
    end
    object AddOffset: TCheckBox
      Left = 8
      Top = 105
      Width = 133
      Height = 17
      Caption = 'Add Offset E/N/U (m)'
      TabOrder = 16
      OnClick = AddOffsetClick
    end
    object Offset1: TEdit
      Left = 144
      Top = 103
      Width = 73
      Height = 21
      TabOrder = 17
      Text = '0'
    end
    object Offset2: TEdit
      Left = 218
      Top = 103
      Width = 73
      Height = 21
      TabOrder = 18
      Text = '0'
    end
    object Offset3: TEdit
      Left = 292
      Top = 103
      Width = 73
      Height = 21
      TabOrder = 19
      Text = '0'
    end
    object OutputTime: TComboBox
      Left = 284
      Top = 78
      Width = 81
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 15
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'GPST'
        'UTC'
        'JST')
    end
    object TimeInt: TEdit
      Left = 98
      Top = 32
      Width = 81
      Height = 21
      TabOrder = 10
      Text = '1'
    end
    object TimeY1: TEdit
      Left = 80
      Top = 8
      Width = 63
      Height = 21
      TabOrder = 1
      Text = '2000/01/01'
    end
    object TimeH1: TEdit
      Left = 157
      Top = 8
      Width = 51
      Height = 21
      TabOrder = 3
      Text = '00:00:00'
    end
    object TimeSpan: TCheckBox
      Left = 8
      Top = 10
      Width = 71
      Height = 17
      Caption = 'Time Span'
      TabOrder = 0
      OnClick = TimeSpanClick
    end
    object TimeY2: TEdit
      Left = 222
      Top = 8
      Width = 63
      Height = 21
      TabOrder = 5
      Text = '2000/01/01'
    end
    object TimeH2: TEdit
      Left = 299
      Top = 8
      Width = 51
      Height = 21
      TabOrder = 7
      Text = '00:00:00'
    end
    object QFlags: TComboBox
      Left = 284
      Top = 32
      Width = 81
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 13
      Text = 'ALL'
      Items.Strings = (
        'ALL'
        'Q=1'
        'Q=2'
        'Q=3'
        'Q=4'
        'Q=5'
        'Q=6')
    end
    object OutputFile: TEdit
      Left = 8
      Top = 168
      Width = 357
      Height = 21
      TabOrder = 23
    end
    object InputFile: TEdit
      Left = 8
      Top = 146
      Width = 357
      Height = 21
      TabOrder = 22
      OnChange = InputFileChange
    end
    object BtnInputFile: TButton
      Left = 349
      Top = 127
      Width = 17
      Height = 17
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 21
      OnClick = BtnInputFileClick
    end
    object Message: TPanel
      Left = 9
      Top = 212
      Width = 357
      Height = 25
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 24
    end
    object TimeY1UD: TUpDown
      Left = 143
      Top = 8
      Width = 13
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 2
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1UD: TUpDown
      Left = 208
      Top = 8
      Width = 13
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 4
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
    object TimeY2UD: TUpDown
      Left = 285
      Top = 8
      Width = 13
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 6
      Wrap = True
      OnChangingEx = TimeY2UDChangingEx
    end
    object TimeH2UD: TUpDown
      Left = 350
      Top = 8
      Width = 13
      Height = 20
      Min = -32000
      Max = 32000
      TabOrder = 8
      Wrap = True
      OnChangingEx = TimeH2UDChangingEx
    end
    object Compress: TCheckBox
      Left = 292
      Top = 129
      Width = 45
      Height = 17
      Caption = '.kmz'
      TabOrder = 20
      OnClick = CompressClick
    end
    object GoogleEarthFile: TEdit
      Left = 8
      Top = 190
      Width = 341
      Height = 21
      TabOrder = 25
      OnChange = GoogleEarthFileChange
    end
    object BtnGoogleEarthFile: TButton
      Left = 349
      Top = 190
      Width = 17
      Height = 20
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 26
      OnClick = BtnGoogleEarthFileClick
    end
  end
  object BtnGoogle: TButton
    Left = 7
    Top = 239
    Width = 79
    Height = 21
    Caption = '&Google Earth'
    TabOrder = 3
    OnClick = BtnGoogleClick
  end
  object OpenDialog: TOpenDialog
    Filter = 'All (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 98
    Top = 216
  end
end
