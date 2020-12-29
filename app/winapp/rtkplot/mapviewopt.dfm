object MapViewOptDialog: TMapViewOptDialog
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Map View Options'
  ClientHeight = 251
  ClientWidth = 490
  Color = clWhite
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 178
    Width = 102
    Height = 13
    Caption = 'Google Maps API Key'
  end
  object Label2: TLabel
    Left = 35
    Top = 26
    Width = 20
    Height = 13
    Caption = 'Title'
  end
  object Label3: TLabel
    Left = 248
    Top = 26
    Width = 61
    Height = 13
    Caption = 'Map Tile URL'
  end
  object Label5: TLabel
    Left = 10
    Top = 10
    Width = 80
    Height = 13
    Caption = 'Leaflet Map Tiles'
  end
  object BtnNotes: TSpeedButton
    Left = 128
    Top = 176
    Width = 23
    Height = 18
    Caption = '?'
    Flat = True
    OnClick = BtnNotesClick
  end
  object BtnOk: TButton
    Left = 312
    Top = 219
    Width = 85
    Height = 29
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 1
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 400
    Top = 219
    Width = 85
    Height = 29
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 0
  end
  object EditApiKey: TEdit
    Left = 4
    Top = 194
    Width = 480
    Height = 21
    TabOrder = 14
  end
  object MapTitle1: TEdit
    Left = 4
    Top = 40
    Width = 85
    Height = 21
    TabOrder = 2
  end
  object MapTile1: TEdit
    Left = 90
    Top = 40
    Width = 393
    Height = 21
    TabOrder = 3
  end
  object MapTitle2: TEdit
    Left = 4
    Top = 62
    Width = 85
    Height = 21
    TabOrder = 4
  end
  object MapTile2: TEdit
    Left = 90
    Top = 62
    Width = 393
    Height = 21
    TabOrder = 5
  end
  object MapTitle3: TEdit
    Left = 4
    Top = 84
    Width = 85
    Height = 21
    TabOrder = 6
  end
  object MapTile3: TEdit
    Left = 90
    Top = 84
    Width = 393
    Height = 21
    TabOrder = 7
  end
  object MapTitle4: TEdit
    Left = 4
    Top = 106
    Width = 85
    Height = 21
    TabOrder = 8
  end
  object MapTile4: TEdit
    Left = 90
    Top = 106
    Width = 393
    Height = 21
    TabOrder = 9
  end
  object MapTitle5: TEdit
    Left = 4
    Top = 128
    Width = 85
    Height = 21
    TabOrder = 10
  end
  object MapTile5: TEdit
    Left = 90
    Top = 128
    Width = 393
    Height = 21
    TabOrder = 11
  end
  object MapTitle6: TEdit
    Left = 4
    Top = 150
    Width = 85
    Height = 21
    TabOrder = 12
  end
  object MapTile6: TEdit
    Left = 90
    Top = 150
    Width = 393
    Height = 21
    TabOrder = 13
  end
end
