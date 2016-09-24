object ConvDialog: TConvDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'KML/GPX Converter'
  ClientHeight = 291
  ClientWidth = 395
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
  object BtnClose: TButton
    Left = 300
    Top = 262
    Width = 89
    Height = 27
    Caption = '&Close'
    TabOrder = 1
    OnClick = BtnCloseClick
  end
  object BtnConvert: TButton
    Left = 204
    Top = 262
    Width = 89
    Height = 27
    Caption = 'Con&vert'
    TabOrder = 0
    OnClick = BtnConvertClick
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 395
    Height = 262
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 4
    object Label1: TLabel
      Left = 10
      Top = 83
      Width = 63
      Height = 13
      Caption = 'Output Track'
    end
    object Label2: TLabel
      Left = 10
      Top = 106
      Width = 83
      Height = 13
      Caption = 'Output Waypoint'
    end
    object Label3: TLabel
      Left = 212
      Top = 84
      Width = 74
      Height = 13
      Caption = 'Output Altitude'
    end
    object Label4: TLabel
      Left = 212
      Top = 107
      Width = 59
      Height = 13
      Caption = 'Output Time'
    end
    object Label5: TLabel
      Left = 212
      Top = 61
      Width = 37
      Height = 13
      Caption = 'Q-Flags'
      FocusControl = AddOffset
    end
    object Label7: TLabel
      Left = 10
      Top = 154
      Width = 121
      Height = 13
      Caption = 'Input/Output/GE Exe File'
    end
    object Label6: TLabel
      Left = 10
      Top = 8
      Width = 71
      Height = 13
      Caption = 'Output Format'
    end
    object TimeIntF: TCheckBox
      Left = 9
      Top = 58
      Width = 83
      Height = 17
      Caption = 'Interval (s)'
      TabOrder = 11
      OnClick = TimeIntFClick
    end
    object TrackColor: TComboBox
      Left = 105
      Top = 80
      Width = 85
      Height = 21
      Style = csDropDownList
      ItemIndex = 5
      TabOrder = 13
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
      Left = 105
      Top = 103
      Width = 85
      Height = 21
      Style = csDropDownList
      ItemIndex = 5
      TabOrder = 14
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
      Left = 302
      Top = 80
      Width = 85
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 16
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'Ellipsoidal'
        'Geodetic')
    end
    object AddOffset: TCheckBox
      Left = 9
      Top = 128
      Width = 133
      Height = 17
      Caption = 'Add Offset E/N/U (m)'
      TabOrder = 18
      OnClick = AddOffsetClick
    end
    object Offset1: TEdit
      Left = 167
      Top = 127
      Width = 73
      Height = 21
      TabOrder = 19
      Text = '0'
    end
    object Offset2: TEdit
      Left = 241
      Top = 127
      Width = 73
      Height = 21
      TabOrder = 20
      Text = '0'
    end
    object Offset3: TEdit
      Left = 315
      Top = 127
      Width = 73
      Height = 21
      TabOrder = 21
      Text = '0'
    end
    object OutputTime: TComboBox
      Left = 302
      Top = 103
      Width = 85
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 17
      Text = 'OFF'
      Items.Strings = (
        'OFF'
        'GPST'
        'UTC'
        'JST')
    end
    object TimeInt: TEdit
      Left = 105
      Top = 57
      Width = 85
      Height = 21
      TabOrder = 12
      Text = '1'
    end
    object TimeY1: TEdit
      Left = 80
      Top = 32
      Width = 63
      Height = 21
      TabOrder = 3
      Text = '2000/01/01'
    end
    object TimeH1: TEdit
      Left = 163
      Top = 32
      Width = 51
      Height = 21
      TabOrder = 5
      Text = '00:00:00'
    end
    object TimeSpan: TCheckBox
      Left = 9
      Top = 34
      Width = 71
      Height = 17
      Caption = 'Time Span'
      TabOrder = 2
      OnClick = TimeSpanClick
    end
    object TimeY2: TEdit
      Left = 235
      Top = 32
      Width = 63
      Height = 21
      TabOrder = 7
      Text = '2000/01/01'
    end
    object TimeH2: TEdit
      Left = 318
      Top = 32
      Width = 51
      Height = 21
      TabOrder = 9
      Text = '00:00:00'
    end
    object QFlags: TComboBox
      Left = 302
      Top = 57
      Width = 85
      Height = 21
      Style = csDropDownList
      ItemIndex = 0
      TabOrder = 15
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
      Top = 192
      Width = 380
      Height = 21
      TabOrder = 25
    end
    object InputFile: TEdit
      Left = 8
      Top = 170
      Width = 380
      Height = 21
      TabOrder = 24
      OnChange = InputFileChange
    end
    object BtnInputFile: TButton
      Left = 364
      Top = 149
      Width = 25
      Height = 21
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 23
      OnClick = BtnInputFileClick
    end
    object Message: TPanel
      Left = 8
      Top = 236
      Width = 381
      Height = 25
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 26
    end
    object TimeY1UD: TUpDown
      Left = 143
      Top = 31
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 4
      Wrap = True
      OnChangingEx = TimeY1UDChangingEx
    end
    object TimeH1UD: TUpDown
      Left = 214
      Top = 31
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 6
      Wrap = True
      OnChangingEx = TimeH1UDChangingEx
    end
    object TimeY2UD: TUpDown
      Left = 298
      Top = 31
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 8
      Wrap = True
      OnChangingEx = TimeY2UDChangingEx
    end
    object TimeH2UD: TUpDown
      Left = 369
      Top = 31
      Width = 19
      Height = 23
      Min = -32000
      Max = 32000
      TabOrder = 10
      Wrap = True
      OnChangingEx = TimeH2UDChangingEx
    end
    object Compress: TCheckBox
      Left = 316
      Top = 152
      Width = 45
      Height = 17
      Caption = '.kmz'
      TabOrder = 22
      OnClick = CompressClick
    end
    object GoogleEarthFile: TEdit
      Left = 8
      Top = 214
      Width = 355
      Height = 21
      TabOrder = 27
      OnChange = GoogleEarthFileChange
    end
    object BtnGoogleEarthFile: TButton
      Left = 364
      Top = 214
      Width = 25
      Height = 21
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 28
      OnClick = BtnGoogleEarthFileClick
    end
    object FormatKML: TRadioButton
      Left = 106
      Top = 7
      Width = 113
      Height = 17
      Caption = 'Google Earth KML'
      Checked = True
      TabOrder = 0
      TabStop = True
      OnClick = FormatKMLClick
    end
    object FormatGPX: TRadioButton
      Left = 240
      Top = 7
      Width = 113
      Height = 17
      Caption = 'GPX'
      TabOrder = 1
      OnClick = FormatGPXClick
    end
  end
  object BtnGoogle: TButton
    Left = 97
    Top = 262
    Width = 89
    Height = 27
    Caption = '&Google Earth'
    TabOrder = 3
    OnClick = BtnGoogleClick
  end
  object BtnView: TBitBtn
    Left = 7
    Top = 262
    Width = 89
    Height = 27
    Caption = '&View...'
    Glyph.Data = {
      3E020000424D3E0200000000000036000000280000000D0000000D0000000100
      1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
      0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
      FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
      FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
      FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
      FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
      0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FF00}
    TabOrder = 2
    OnClick = BtnViewClick
  end
  object OpenDialog: TOpenDialog
    Filter = 'All (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 138
    Top = 221
  end
end
