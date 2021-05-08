object VecMapDialog: TVecMapDialog
  Left = 0
  Top = 5
  BorderStyle = bsDialog
  Caption = 'Map Layer'
  ClientHeight = 335
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
  object BtnApply: TButton
    Left = 234
    Top = 302
    Width = 84
    Height = 29
    Caption = '&Apply'
    TabOrder = 0
    OnClick = BtnApplyClick
  end
  object BtnClose: TButton
    Left = 321
    Top = 302
    Width = 74
    Height = 29
    Caption = '&Close'
    ModalResult = 8
    TabOrder = 1
    OnClick = BtnCloseClick
  end
  object BtnUp: TBitBtn
    Left = 4
    Top = 302
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
    Left = 70
    Top = 302
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
      Left = 326
      Top = 3
      Width = 19
      Height = 13
      Caption = 'Line'
    end
    object Label3: TLabel
      Left = 10
      Top = 3
      Width = 26
      Height = 13
      Caption = 'Show'
    end
    object Label4: TLabel
      Left = 365
      Top = 3
      Width = 12
      Height = 13
      Caption = 'Fill'
    end
    object Panel21: TPanel
      Left = 4
      Top = 18
      Width = 390
      Height = 280
      AutoSize = True
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
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 0
        object Color1: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis1: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer1: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color1F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel12: TPanel
        Left = 2
        Top = 25
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 1
        object Color2: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis2: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer2: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color2F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel13: TPanel
        Left = 2
        Top = 48
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 2
        object Color3: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis3: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer3: TRadioButton
          Left = 33
          Top = 4
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color3F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel14: TPanel
        Left = 2
        Top = 71
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 3
        object Color4: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis4: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer4: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color4F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel15: TPanel
        Left = 2
        Top = 94
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 4
        ExplicitTop = 95
        object Color5: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis5: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer5: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color5F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel16: TPanel
        Left = 2
        Top = 117
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 5
        ExplicitTop = 118
        object Color6: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis6: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer6: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color6F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel17: TPanel
        Left = 2
        Top = 140
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 6
        ExplicitTop = 141
        object Color7: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis7: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer7: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color7F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel18: TPanel
        Left = 2
        Top = 163
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 7
        ExplicitTop = 164
        object Color8: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis8: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer8: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color8F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel19: TPanel
        Left = 2
        Top = 186
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 8
        ExplicitTop = 187
        object Color9: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis9: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer9: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color9F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel1A: TPanel
        Left = 2
        Top = 209
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 9
        ExplicitTop = 210
        object Color10: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis10: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer10: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color10F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel1B: TPanel
        Left = 2
        Top = 232
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 10
        ExplicitTop = 233
        object Color11: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis11: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer11: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color11F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
      object Panel22: TPanel
        Left = 2
        Top = 255
        Width = 386
        Height = 23
        Align = alTop
        BevelOuter = bvLowered
        Color = clWindow
        ParentBackground = False
        TabOrder = 11
        ExplicitTop = 256
        object Color12: TPanel
          Left = 313
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 0
          OnClick = ColorClick
          ExplicitTop = 0
          ExplicitHeight = 23
        end
        object Vis12: TCheckBox
          Left = 8
          Top = 3
          Width = 19
          Height = 17
          TabOrder = 1
          OnClick = Vis1Click
        end
        object Layer12: TRadioButton
          Left = 32
          Top = 3
          Width = 278
          Height = 17
          TabOrder = 2
          OnClick = Layer1Click
        end
        object Color12F: TPanel
          Left = 349
          Top = 1
          Width = 36
          Height = 21
          Align = alRight
          BevelInner = bvRaised
          BevelOuter = bvLowered
          Color = clWhite
          ParentBackground = False
          TabOrder = 3
          OnClick = ColorClick
          ExplicitLeft = 350
          ExplicitTop = 0
          ExplicitHeight = 23
        end
      end
    end
  end
  object ColorDialog: TColorDialog
    Options = [cdFullOpen]
    Left = 182
    Top = 147
  end
end
