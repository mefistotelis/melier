unit ExportDlg;

{$R-}

interface

uses Messages, Windows, SysUtils, Classes, Controls, StdCtrls, Graphics,
  ExtCtrls, Buttons, Dialogs;

type

{ TExportDialog }

  TExportDialog = class(TOpenDialog)
  private
    ExpSourcePanel: TPanel;
  protected
    procedure DoClose; override;
    procedure DoShow; override;
  public
    ExpCurrent: TRadioButton;
    ExpNewSearch: TRadioButton;
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    function Execute: Boolean; override;
  end;

implementation

uses Consts, Forms, CommDlg, Dlgs;

{ TExportDialog }

{$R ExportDlgX.RES}

constructor TExportDialog.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  Filter:='Plik tekstowy (*.txt)|*.TXT|Rich Text Format (*.rtf)|*.RTF';
  ExpSourcePanel := TPanel.Create(Self);
  with ExpSourcePanel do
  begin
    Name := 'ExpSourcePanel';
    Caption := '';
    SetBounds(204, 5, 169, 200);
    BevelOuter := bvNone;
    BorderWidth := 6;
    TabOrder := 1;
    ExpCurrent := TRadioButton.Create(Self);
    with ExpCurrent do begin
      Name := 'ExpCurrent';
      Caption := 'Eksportuj przez zapis zawartoœci okna rezultatów';
      SetBounds(6, 6, 157, 23);
      Align := alTop;
      Parent := ExpSourcePanel;
    end;
    ExpNewSearch := TradioButton.Create(Self);
    with ExpNewSearch do begin
      Name := 'ExpNewSearch';
      Caption := 'Eksportuj bezpoœrednio z procedury wyszukuj¹cej';
      SetBounds(6, 6, 157, 23);
      Align := alTop;
      Parent := ExpSourcePanel;
    end;
    ExpCurrent.Checked:=True;
    ExpNewSearch.Enabled:=False;
  end;
end;

destructor TExportDialog.Destroy;
begin
  ExpNewSearch.Free;
  ExpCurrent.Free;
  ExpSourcePanel.Free;
  inherited Destroy;
end;

procedure TExportDialog.DoClose;
begin
  inherited DoClose;
  { Hide any hint windows left behind }
  Application.HideHint;
end;

procedure TExportDialog.DoShow;
var
  PreviewRect, StaticRect: TRect;
begin
  { Set preview area to entire dialog }
  GetClientRect(Handle, PreviewRect);
  StaticRect := GetStaticRect;
  { Move preview area to right of static area }
  PreviewRect.Left :=4;
  PreviewRect.Top:=StaticRect.Bottom-7;
  ExpSourcePanel.BoundsRect := PreviewRect;
  ExpSourcePanel.ParentWindow := Handle;
  inherited DoShow;
end;

function TExportDialog.Execute;
begin
  if NewStyleControls and not (ofOldStyleDialog in Options) then
    Template := 'EXDLGTEMP' else
    Template := nil;
  Result := DoExecute(@GetSaveFileName);
end;

end.

