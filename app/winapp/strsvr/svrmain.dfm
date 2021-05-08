object MainForm: TMainForm
  Left = 0
  Top = 0
  BorderIcons = []
  Caption = 'STRSVR'
  ClientHeight = 232
  ClientWidth = 458
  Color = clWhite
  Constraints.MaxHeight = 358
  Constraints.MaxWidth = 474
  Constraints.MinHeight = 188
  Constraints.MinWidth = 474
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel5: TPanel
    AlignWithMargins = True
    Left = 1
    Top = 201
    Width = 456
    Height = 30
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alBottom
    BevelOuter = bvNone
    Caption = 'Panel5'
    TabOrder = 0
    object BtnExit: TBitBtn
      AlignWithMargins = True
      Left = 304
      Top = 0
      Width = 151
      Height = 29
      Margins.Left = 1
      Margins.Top = 0
      Margins.Right = 1
      Margins.Bottom = 1
      Align = alRight
      Caption = 'E&xit'
      TabOrder = 0
      OnClick = BtnExitClick
    end
    object BtnOpt: TBitBtn
      AlignWithMargins = True
      Left = 307
      Top = 0
      Width = 0
      Height = 29
      Margins.Left = 1
      Margins.Top = 0
      Margins.Right = 1
      Margins.Bottom = 1
      Align = alClient
      Caption = '&Options...'
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F7F7F0000007F7F7FFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFC3C3C3000000C3C3C37F7F7F0000007F
        7F7FC3C3C3000000C3C3C3FFFFFFFFFFFF00FFFFFFFFFFFF0000000000000000
        00000000000000000000000000000000000000FFFFFFFFFFFF00FFFFFFFFFFFF
        C3C3C30000000000007F7F7F7F7F7F7F7F7F000000000000C3C3C3FFFFFFFFFF
        FF00FFFFFF7F7F7F7F7F7F0000007F7F7FC3C3C3FFFFFFC3C3C37F7F7F000000
        7F7F7F7F7F7FFFFFFF00FFFFFF0000000000000000007F7F7FFFFFFFFFFFFFFF
        FFFF7F7F7F000000000000000000FFFFFF00FFFFFF7F7F7F7F7F7F0000007F7F
        7FC3C3C3FFFFFFC3C3C37F7F7F0000007F7F7F7F7F7FFFFFFF00FFFFFFFFFFFF
        C3C3C30000000000007F7F7F7F7F7F7F7F7F000000000000C3C3C3FFFFFFFFFF
        FF00FFFFFFFFFFFF000000000000000000000000000000000000000000000000
        000000FFFFFFFFFFFF00FFFFFFFFFFFFC3C3C3000000C3C3C37F7F7F0000007F
        7F7FC3C3C3000000C3C3C3FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF7F7F7F0000007F7F7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      TabOrder = 1
      OnClick = BtnOptClick
    end
    object BtnStart: TBitBtn
      AlignWithMargins = True
      Left = 1
      Top = 0
      Width = 151
      Height = 29
      Margins.Left = 1
      Margins.Top = 0
      Margins.Right = 1
      Margins.Bottom = 1
      Align = alLeft
      Caption = '&Start'
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF008000008000FFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF0080000080000080
        00008000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        008000008000008000008000008000008000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFF008000008000008000008000008000008000008000008000
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF00800000800000800000800000800000
        8000008000008000008000FFFFFFFFFFFF00FFFFFFFFFFFF0080000080000080
        00008000008000008000008000008000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        008000008000008000008000008000008000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFF008000008000008000008000FFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF008000008000FFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      TabOrder = 2
      OnClick = BtnStartClick
    end
    object BtnStop: TBitBtn
      AlignWithMargins = True
      Left = 154
      Top = 0
      Width = 151
      Height = 29
      Margins.Left = 1
      Margins.Top = 0
      Margins.Right = 1
      Margins.Bottom = 1
      Align = alLeft
      Caption = 'S&top'
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF0000000000
        00000000000000000000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFF000000000000000000000000000000000000000000FFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF00000000000000000000000000
        0000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF0000000000
        00000000000000000000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFF000000000000000000000000000000000000000000FFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      TabOrder = 3
      Visible = False
      OnClick = BtnStopClick
    end
  end
  object Panel1: TPanel
    AlignWithMargins = True
    Left = 1
    Top = 29
    Width = 456
    Height = 132
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alClient
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    ExplicitHeight = 133
    object Panel12: TPanel
      Left = 2
      Top = 22
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 0
      object InputBps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object InputByte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object LabelInput: TLabel
        Left = 25
        Top = 7
        Width = 43
        Height = 13
        Caption = '(0) Input'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnInput: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnInputClick
      end
      object IndInput: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 2
      end
      object Input: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 3
        OnChange = InputChange
        Items.Strings = (
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Client'
          'UDP Server'
          'File')
      end
      object BtnLog: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 4
        OnClick = BtnLogClick
      end
      object IndLog: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 5
      end
    end
    object Panel11: TPanel
      Left = 2
      Top = 2
      Width = 452
      Height = 20
      Align = alTop
      TabOrder = 1
      object Label1: TLabel
        Left = 199
        Top = 2
        Width = 21
        Height = 13
        Caption = 'Cmd'
      end
      object Label2: TLabel
        Left = 225
        Top = 2
        Width = 25
        Height = 13
        Caption = 'Conv'
      end
      object Label3: TLabel
        Left = 115
        Top = 2
        Width = 24
        Height = 13
        Caption = 'Type'
      end
      object Label4: TLabel
        Left = 175
        Top = 2
        Width = 18
        Height = 13
        Caption = 'Opt'
      end
      object Label5: TLabel
        Left = 35
        Top = 2
        Width = 34
        Height = 13
        Caption = 'Stream'
      end
      object Label6: TLabel
        Left = 350
        Top = 2
        Width = 27
        Height = 13
        Caption = 'Bytes'
      end
      object Label7: TLabel
        Left = 426
        Top = 2
        Width = 17
        Height = 13
        Caption = 'Bps'
      end
      object Label9: TLabel
        Left = 263
        Top = 2
        Width = 17
        Height = 13
        Caption = 'Log'
      end
    end
    object Panel13: TPanel
      Left = 2
      Top = 50
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 2
      object LabelOutput1: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(1) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output1Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output1Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd1: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv1: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput1: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput1: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
      end
      object Output1: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog1: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog1: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
    object Panel14: TPanel
      Left = 2
      Top = 78
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 3
      object LabelOutput2: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(2) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output2Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output2Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd2: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv2: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput2: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput2: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
        OnClick = BtnOutputClick
      end
      object Output2: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog2: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog2: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
    object Panel17: TPanel
      Left = 2
      Top = 162
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 4
      object LabelOutput5: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(5) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output5Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output5Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd5: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv5: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput5: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput5: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
        OnClick = BtnOutputClick
      end
      object Output5: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog5: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog5: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
    object Panel16: TPanel
      Left = 2
      Top = 134
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 5
      object LabelOutput4: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(4) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output4Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output4Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd4: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv4: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput4: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput4: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
        OnClick = BtnOutputClick
      end
      object Output4: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog4: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog4: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
    object Panel15: TPanel
      Left = 2
      Top = 106
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 6
      object LabelOutput3: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(3) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output3Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output3Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd3: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv3: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput3: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput3: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
      end
      object Output3: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog3: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog3: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
    object Panel18: TPanel
      Left = 2
      Top = 190
      Width = 452
      Height = 28
      Align = alTop
      TabOrder = 7
      object LabelOutput6: TLabel
        Left = 24
        Top = 7
        Width = 51
        Height = 13
        Caption = '(6) Output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output6Bps: TLabel
        Left = 378
        Top = 7
        Width = 65
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object Output6Byte: TLabel
        Left = 292
        Top = 7
        Width = 85
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
      end
      object BtnCmd6: TButton
        Left = 198
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnCmdClick
      end
      object BtnConv6: TButton
        Left = 224
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnConvClick
      end
      object BtnOutput6: TButton
        Left = 172
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutputClick
      end
      object IndOutput6: TPanel
        Left = 6
        Top = 8
        Width = 12
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 3
        OnClick = BtnOutputClick
      end
      object Output6: TComboBox
        Left = 82
        Top = 3
        Width = 88
        Height = 21
        Style = csDropDownList
        DropDownCount = 16
        TabOrder = 4
        OnChange = OutputChange
        Items.Strings = (
          ''
          'Serial'
          'TCP Client'
          'TCP Server'
          'NTRIP Server'
          'NTRIP Caster'
          'UDP Client'
          'File')
      end
      object BtnLog6: TButton
        Left = 250
        Top = 2
        Width = 25
        Height = 23
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnLogClick
      end
      object IndLog6: TPanel
        Left = 280
        Top = 8
        Width = 8
        Height = 12
        BevelInner = bvRaised
        BevelOuter = bvLowered
        ParentBackground = False
        TabOrder = 6
      end
    end
  end
  object Panel2: TPanel
    AlignWithMargins = True
    Left = 1
    Top = 1
    Width = 456
    Height = 26
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alTop
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 2
    object Label8: TLabel
      Left = 275
      Top = 5
      Width = 81
      Height = 14
      Caption = 'Connect Time:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -12
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object ConTime: TLabel
      Left = 376
      Top = 5
      Width = 68
      Height = 14
      Alignment = taRightJustify
      Caption = '0d 00:00:00'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -12
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Time: TLabel
      Left = 10
      Top = 5
      Width = 154
      Height = 14
      Caption = '2010/01/01 00:00:00 GPST'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -12
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
  end
  object Panel4: TPanel
    AlignWithMargins = True
    Left = 1
    Top = 174
    Width = 456
    Height = 25
    Margins.Left = 1
    Margins.Top = 2
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alBottom
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 3
    object Message: TLabel
      Left = 21
      Top = 2
      Width = 395
      Height = 21
      Align = alClient
      Alignment = taCenter
      AutoSize = False
      Caption = 'message area'
      Color = clBtnFace
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentColor = False
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      Layout = tlCenter
      ExplicitTop = 3
      ExplicitWidth = 343
    end
    object BtnAbout: TSpeedButton
      Left = 435
      Top = 2
      Width = 19
      Height = 21
      Align = alRight
      Caption = '?'
      Flat = True
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      OnClick = BtnAboutClick
      ExplicitLeft = 381
      ExplicitTop = 1
      ExplicitHeight = 23
    end
    object BtnStrMon: TSpeedButton
      Left = 2
      Top = 2
      Width = 19
      Height = 21
      Hint = 'Stream Monitor'
      Align = alLeft
      Flat = True
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF8080808080
        80808080808080808080808080808080FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFF808080FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF808080
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFF808080FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF808080FFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFF808080FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFF808080808080808080808080808080808080808080
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      ParentShowHint = False
      ShowHint = True
      OnClick = BtnStrMonClick
      ExplicitLeft = 3
      ExplicitHeight = 50
    end
    object BtnTaskIcon: TSpeedButton
      Left = 416
      Top = 2
      Width = 19
      Height = 21
      Hint = 'Task Tray Icon'
      Align = alRight
      Flat = True
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      Glyph.Data = {
        3E020000424D3E0200000000000036000000280000000D0000000D0000000100
        1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF787878FFFFFF787878FFFFFF787878FF
        FFFF000000000000000000FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFF000000FFFFFF000000FFFFFFFFFFFF00FFFFFFFFFFFF
        787878FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000FFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF787878FFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFF787878FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        787878FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF787878FFFFFFFFFF
        FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF787878FFFFFF787878FFFFFF787878FF
        FFFF787878FFFFFF787878FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
        FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        FF00}
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      OnClick = BtnTaskIconClick
      ExplicitLeft = 358
      ExplicitHeight = 19
    end
  end
  object Progress: TProgressBar
    AlignWithMargins = True
    Left = 2
    Top = 164
    Width = 454
    Height = 7
    Margins.Left = 2
    Margins.Top = 2
    Margins.Right = 2
    Margins.Bottom = 1
    Align = alBottom
    Smooth = True
    MarqueeInterval = 0
    Step = 1
    TabOrder = 4
    ExplicitTop = 165
  end
  object Timer1: TTimer
    Interval = 50
    OnTimer = Timer1Timer
    Left = 213
    Top = 249
  end
  object Timer2: TTimer
    Interval = 100
    OnTimer = Timer2Timer
    Left = 242
    Top = 249
  end
  object PopupMenu: TPopupMenu
    Left = 332
    Top = 248
    object MenuExpand: TMenuItem
      Caption = 'E&xpand'
      OnClick = MenuExpandClick
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object MenuStart: TMenuItem
      Caption = '&Start'
      OnClick = MenuStartClick
    end
    object MenuStop: TMenuItem
      Caption = '&Stop'
      Enabled = False
      OnClick = MenuStopClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object MenuExit: TMenuItem
      Caption = '&Exit'
      OnClick = MenuExitClick
    end
  end
  object TrayIcon: TTrayIcon
    Hint = 'Stream Server'
    Icon.Data = {
      0000010001001010000001000800680500001600000028000000100000002000
      0000010008000000000040010000000000000000000000000000000000000000
      0000000080000080000000808000800000008000800080800000C0C0C000C0DC
      C000F0CAA6000020400000206000002080000020A0000020C0000020E0000040
      0000004020000040400000406000004080000040A0000040C0000040E0000060
      0000006020000060400000606000006080000060A0000060C0000060E0000080
      0000008020000080400000806000008080000080A0000080C0000080E00000A0
      000000A0200000A0400000A0600000A0800000A0A00000A0C00000A0E00000C0
      000000C0200000C0400000C0600000C0800000C0A00000C0C00000C0E00000E0
      000000E0200000E0400000E0600000E0800000E0A00000E0C00000E0E0004000
      0000400020004000400040006000400080004000A0004000C0004000E0004020
      0000402020004020400040206000402080004020A0004020C0004020E0004040
      0000404020004040400040406000404080004040A0004040C0004040E0004060
      0000406020004060400040606000406080004060A0004060C0004060E0004080
      0000408020004080400040806000408080004080A0004080C0004080E00040A0
      000040A0200040A0400040A0600040A0800040A0A00040A0C00040A0E00040C0
      000040C0200040C0400040C0600040C0800040C0A00040C0C00040C0E00040E0
      000040E0200040E0400040E0600040E0800040E0A00040E0C00040E0E0008000
      0000800020008000400080006000800080008000A0008000C0008000E0008020
      0000802020008020400080206000802080008020A0008020C0008020E0008040
      0000804020008040400080406000804080008040A0008040C0008040E0008060
      0000806020008060400080606000806080008060A0008060C0008060E0008080
      0000808020008080400080806000808080008080A0008080C0008080E00080A0
      000080A0200080A0400080A0600080A0800080A0A00080A0C00080A0E00080C0
      000080C0200080C0400080C0600080C0800080C0A00080C0C00080C0E00080E0
      000080E0200080E0400080E0600080E0800080E0A00080E0C00080E0E000C000
      0000C0002000C0004000C0006000C0008000C000A000C000C000C000E000C020
      0000C0202000C0204000C0206000C0208000C020A000C020C000C020E000C040
      0000C0402000C0404000C0406000C0408000C040A000C040C000C040E000C060
      0000C0602000C0604000C0606000C0608000C060A000C060C000C060E000C080
      0000C0802000C0804000C0806000C0808000C080A000C080C000C080E000C0A0
      0000C0A02000C0A04000C0A06000C0A08000C0A0A000C0A0C000C0A0E000C0C0
      0000C0C02000C0C04000C0C06000C0C08000C0C0A000F0FBFF00A4A0A0008080
      80000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF000707
      070707070707070707070707070707FFFFFFFFFFFFFFFFFF00000000000007FF
      FFFFFFFFFFFFFFA400FFFFFFFF0007FFFFFFFFFFFFA4A4A4A4FFFFFFFF0007FF
      FFFFFFFFFFA4FFA400FFFFFFFF00000000000000FFA4FFFF00000000000000FF
      FFFFFF00FFA4FFA400FFFFFFFF0000FFFFFFFF00A4A4A4A4A4FFFFFFFF0000FF
      FFFFFF00FFA4FFA400FFFFFFFF00000000000000FFA4FFFF00000000000007FF
      FFFFFFFFFFA4FFA400FFFFFFFF0007FFFFFFFFFFFFA4A4A4A4FFFFFFFF0007FF
      FFFFFFFFFFFFFFA400FFFFFFFF0007FFFFFFFFFFFFFFFFFF0000000000000707
      0707070707070707070707070707000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000FFFF0000}
    PopupMenu = PopupMenu
    OnDblClick = TrayIconDblClick
    OnMouseDown = TrayIconMouseDown
    Left = 301
    Top = 248
  end
  object ImageList: TImageList
    Left = 271
    Top = 248
    Bitmap = {
      494C010103000400040010001000FFFFFFFFFF10FFFFFFFFFFFFFFFF424D3600
      0000000000003600000028000000400000001000000001002000000000000010
      000000000000000000000000000000000000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000B4B4B400B4B4B400B4B4B400B4B4
      B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4
      B400B4B4B400B4B4B400B4B4B400B4B4B4000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      000000000000000000000000000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF0080808000000000000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000808080008080800080808000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00808080008080800080808000808080000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF007878780078787800787878007878780080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000FFFFFF0080808000000000000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0078787800FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0080808000FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0080808000FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0078787800FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF0000000000FFFFFF0080808000FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00000000000000000000800000008000000080
      00000080000000000000FFFFFF0080808000FFFFFF0080808000000000000080
      0000008000000080000000800000000000000000000080FF000080FF000080FF
      000080FF000000000000FFFFFF0078787800FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00000000008080800080808000808080008080800080808000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00000000000000000000800000008000000080
      0000008000000000000080808000808080008080800080808000808080000080
      0000008000000080000000800000000000000000000080FF000080FF000080FF
      000080FF000000000000787878007878780078787800787878007878780080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000000000000000000000000000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF0000000000FFFFFF0080808000FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00000000000000000000800000008000000080
      00000080000000000000FFFFFF0080808000FFFFFF0080808000000000000080
      0000008000000080000000800000000000000000000080FF000080FF000080FF
      000080FF000000000000FFFFFF0078787800FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0080808000FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0080808000FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00000000000000000000FFFFFF0078787800FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000FFFFFF0080808000000000000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0078787800FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0080808000808080008080800080808000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00808080008080800080808000808080000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF007878780078787800787878007878780080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF008080800000000000FFFF
      FF00FFFFFF00FFFFFF00FFFFFF0000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF0080808000000000000080
      000000800000008000000080000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00787878000000000080FF
      000080FF000080FF000080FF0000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      000000000000000000000000000000000000C0C0C000FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      000000000000000000000000000000000000B4B4B400FFFFFF00FFFFFF00FFFF
      FF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0C000C0C0
      C000C0C0C000C0C0C000C0C0C000C0C0C000B4B4B400B4B4B400B4B4B400B4B4
      B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4B400B4B4
      B400B4B4B400B4B4B400B4B4B400B4B4B4000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      000000000000000000000000000000000000424D3E000000000000003E000000
      2800000040000000100000000100010000000000800000000000000000000000
      000000000000000000000000FFFFFF0000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000000000000000000000000000000000
      0000000000000000FFFFFFFFFFFF000000000000000000000000000000000000
      000000000000}
  end
end
