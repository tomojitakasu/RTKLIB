object VecMapDialog: TVecMapDialog
  Left = 0
  Top = 5
  BorderStyle = bsDialog
  Caption = 'Map Layer'
  ClientHeight = 333
  ClientWidth = 398
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
    Left = 222
    Top = 301
    Width = 84
    Height = 30
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 0
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 309
    Top = 301
    Width = 84
    Height = 30
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object BtnUp: TBitBtn
    Left = 12
    Top = 301
    Width = 63
    Height = 29
    Caption = '&Up'
    Glyph.Data = {
      DE000000424DDE00000000000000360000002800000007000000070000000100
      180000000000A800000000000000000000000000000000000000FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFF000000FFFFFF000000000000000000000000000000FFFFFF00
      0000FFFFFFFFFFFF000000000000000000FFFFFFFFFFFF000000FFFFFFFFFFFF
      FFFFFF000000FFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
      0000}
    TabOrder = 2
    OnClick = BtnUpClick
  end
  object BtnDown: TBitBtn
    Left = 76
    Top = 301
    Width = 63
    Height = 29
    Caption = '&Down'
    Glyph.Data = {
      DE000000424DDE00000000000000360000002800000007000000070000000100
      180000000000A800000000000000000000000000000000000000FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFF00
      0000FFFFFFFFFFFF000000000000000000FFFFFFFFFFFF000000FFFFFF000000
      000000000000000000000000FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00
      0000}
    TabOrder = 3
    OnClick = BtnDownClick
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 398
    Height = 300
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 4
    object Label1: TLabel
      Left = 160
      Top = 3
      Width = 27
      Height = 13
      Caption = 'Layer'
    end
    object Label2: TLabel
      Left = 335
      Top = 3
      Width = 25
      Height = 13
      Caption = 'Color'
    end
    object Label3: TLabel
      Left = 10
      Top = 3
      Width = 26
      Height = 13
      Caption = 'Show'
    end
    object Panel21: TPanel
      Left = 4
      Top = 18
      Width = 390
      Height = 281
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Caption = 'Panel21'
      Color = clWindow
      ParentBackground = False
      TabOrder = 0
      object Panel11: TPanel
        Left = 2
        Top = 2
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 0
        object Color1: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor1: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor1Click
        end
        object Vis1: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer1: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel2: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel12: TPanel
        Left = 2
        Top = 25
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 1
        object Color2: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor2: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor2Click
        end
        object Vis2: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer2: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel3: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel13: TPanel
        Left = 2
        Top = 48
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 2
        object Color3: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor3: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor3Click
        end
        object Vis3: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer3: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel4: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel14: TPanel
        Left = 2
        Top = 71
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 3
        object Color4: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor4: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor4Click
        end
        object Vis4: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer4: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel5: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel15: TPanel
        Left = 2
        Top = 95
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 4
        object Color5: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor5: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor5Click
        end
        object Vis5: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer5: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel6: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel16: TPanel
        Left = 2
        Top = 118
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 5
        object Color6: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor6: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor6Click
        end
        object Vis6: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer6: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel7: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel17: TPanel
        Left = 2
        Top = 141
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 6
        object Color7: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor7: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor7Click
        end
        object Vis7: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer7: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel8: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel18: TPanel
        Left = 2
        Top = 164
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 7
        object Color8: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor8: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor8Click
        end
        object Vis8: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer8: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel9: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel19: TPanel
        Left = 2
        Top = 187
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 8
        object Color9: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor9: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor9Click
        end
        object Vis9: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer9: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel10: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel1A: TPanel
        Left = 2
        Top = 210
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 9
        object Color10: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor10: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor10Click
        end
        object Vis10: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer10: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel20: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel1B: TPanel
        Left = 2
        Top = 233
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 10
        object Color11: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor11: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor11Click
        end
        object Vis11: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer11: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
        object Panel24: TPanel
          Left = 6
          Top = 22
          Width = 304
          Height = 1
          BevelInner = bvLowered
          TabOrder = 4
        end
      end
      object Panel22: TPanel
        Left = 2
        Top = 256
        Width = 386
        Height = 23
        BevelOuter = bvNone
        Color = clWindow
        ParentBackground = False
        TabOrder = 11
        object Color12: TPanel
          Left = 311
          Top = 0
          Width = 48
          Height = 23
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
        end
        object BtnColor12: TButton
          Left = 358
          Top = 0
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = BtnColor12Click
        end
        object Vis12: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 2
          OnClick = Vis1Click
        end
        object Layer12: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 3
          OnClick = Layer1Click
        end
      end
    end
  end
  object ColorDialog: TColorDialog
    Options = [cdFullOpen]
    Left = 115
    Top = 295
  end
end
