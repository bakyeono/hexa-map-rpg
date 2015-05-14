#include "LocalStage.h"

using namespace draw;
using namespace sound;
using namespace rule;

// 생성자 및 소멸자
LocalStage::LocalStage()
    : place(128, 128),
      revealed_place(128, 128),
      camera_pos(0, 0),
      crets(),
      quit(false),
      player_id(0),
      ai_distance_limit(50),
      ai_distance_slow(35) {
}

LocalStage::~LocalStage() {
}

void LocalStage::UpdateRevealedPlace() {
  Coord player_pos(crets[player_id].pos());
  // 현재 처리중인 셀의 좌표
  Coord current_cell;
  // 시야범위 체크 시작지점. 여기서 + max_sight_range*2 까지 체크한다.
  int sight_start_x = player_pos.x() - max_sight_range;
  int sight_start_y = player_pos.y() - max_sight_range;
  // 맵의 y 축 루프
  for (int y = 0; y <= max_sight_range*2; ++y) {
    current_cell.set_y(sight_start_y + y);
    if (current_cell.y() < 0 || current_cell.y() >= place.GetRows())
      continue;
    if (current_cell.y() < player_pos.y()-max_sight_range ||
        current_cell.y() > player_pos.y()+max_sight_range)
      continue;
    for (int x = 0; x <= max_sight_range*2; ++x) {
      current_cell.set_x(sight_start_x + x);
      // 범위초과시 continue
      if (current_cell.x() < 0 || current_cell.x() >= place.GetCols())
        continue;
      if (current_cell.x() < player_pos.x()-max_sight_range ||
          current_cell.x() > player_pos.x()+max_sight_range)
        continue;
      // 시야선 검사
      if (crets[player_id].SightAt(Coord(current_cell.x()-sight_start_x,
                                         current_cell.y()-sight_start_y))) {
        // 밝혀진 지도 업데이트
        revealed_place.SetHeight(current_cell, place.GetHeight(current_cell));
        revealed_place.SetTerrain(current_cell, place.GetTerrain(current_cell));
        revealed_place.SetObject(current_cell, place.GetObject(current_cell));
      }
    }
  }
}

void LocalStage::RevealRevealedPlace() {
  HexPos pos;
  for (int y = 0; y<place.GetRows(); ++y) {
    pos.set_y(y);
    for (int x = 0; x<place.GetCols(); ++x) {
      pos.set_x(x);
      revealed_place.SetHeight(pos, place.GetHeight(pos));
      revealed_place.SetTerrain(pos, place.GetTerrain(pos));
      revealed_place.SetObject(pos, place.GetObject(pos));
    }
  }
}

void LocalStage::ConcealRevealedPlace() {
  HexPos pos;
  for (int y = 0; y<place.GetRows(); ++y) {
    pos.set_y(y);
    for (int x = 0; x<place.GetCols(); ++x) {
      pos.set_x(x);
      revealed_place.SetHeight(pos, 0);
      revealed_place.SetTerrain(pos, -1);
      revealed_place.SetObject(pos, -1);
    }
  }
}

void LocalStage::SetCameraPos(const Coord& pos) {
  camera_pos = pos;
}


//const bool LocalStage::Load(const string& filename) {
//    Clear();
//
//    ifstream f(filename.c_str(), ios_base::in | ios_base::binary);
//    if (!f.is_open()) {
//        cerr << filename.c_str() << " 파일을 읽을 수 없습니다." << endl;
//    }
//
//    f.read((char*)&cols, sizeof(cols));
//    f.read((char*)&rows, sizeof(rows));
//
//    int terrain = -1;
//    int wall = -1;
//    int object_id = -1;
//
//    for (int y = 0; y < rows; ++y) {
//        for (int x = 0; x < cols; ++x) {
//            f.read((char*)&terrainType, sizeof(terrainType));
//            f.read((char*)&terrainTile.set, sizeof(terrainTile.set));
//            f.read((char*)&terrainTile.id, sizeof(terrainTile.id));
//            f.read((char*)&wallType, sizeof(terrainType));
//            f.read((char*)&wallTile.set, sizeof(terrainTile.set));
//            f.read((char*)&wallTile.id, sizeof(terrainTile.id));
//            f.read((char*)&object_id, sizeof(object_id));
//            cells.push_back(new LocalPlaceCell(terrainType, terrainTile, wallType, wallTile, object_id));
//        }
//    }
//    f.close();
//    return true;
//}
//const bool LocalStage::Save(const string& filename) {
//    ofstream f(filename.c_str(), ios_base::out | ios_base::trunc | ios_base::binary);
//
//    f.write((char*)&cols, sizeof(cols));
//    f.write((char*)&rows, sizeof(rows));
//
//    int terrain = -1;
//    int wall = -1;
//    int object_id = -1;
//
//    for (int y = 0; y < rows; ++y) {
//        for (int x = 0; x < cols; ++x) {
//            terrainType = cells[y*cols+x]->terrainType;
//            terrainTile = cells[y*cols+x]->terrainTile;
//            wallType = cells[y*cols+x]->wallType;
//            wallTile = cells[y*cols+x]->wallTile;
//            object_id = cells[y*cols+x]->object_id;
//            f.write((char*)&terrainType, sizeof(terrainType));
//            f.write((char*)&terrainTile.set, sizeof(terrainTile.set));
//            f.write((char*)&terrainTile.id, sizeof(terrainTile.id));
//            f.write((char*)&wallType, sizeof(wallType));
//            f.write((char*)&wallTile.set, sizeof(wallTile.set));
//            f.write((char*)&wallTile.id, sizeof(wallTile.id));
//            f.write((char*)&object_id, sizeof(object_id));
//        }
//    }
//    f.close();
//    return true;
//}

int LocalStage::GetSideLevel(Coord& pos) const {
  HexPos src(pos);
  HexPos tar(pos);
  const int def(0);
  int sw;
  int se;
  int level;
  tar.MoveTo(HexDir::swest);
  if (place.IsOutOfRange(tar))
    return def;
  sw = place.GetHeight(src) - place.GetHeight(tar);
  tar.MoveTo(HexDir::east);
  if (place.IsOutOfRange(tar))
    return sw+def;
  se = place.GetHeight(src) - place.GetHeight(tar);
  level = (sw>se) ? sw+def : se+def;
  if (level<def)
    level = def;
  return level;
}

void LocalStage::DrawPlace() {
  int player_id = 0;

  // 기준점의 y축 좌표 가 짝수인지 저장하는 변수.
  // y축이 홀수냐 짝수냐에 따라 x축 헥사맵 출력 방법이 달라져야 함.
  int is_y_odd(camera_pos.y()%2);
  int camera_height = place.GetHeight(camera_pos);

  // 지도의 출력 시작 위치. 여기서 + 범위좌표(cols, rows)까지 출력한다.
  int xStart = camera_pos.x() - frame_place_cols/2;
  int yStart = camera_pos.y() - frame_place_rows/2;

  // 현재 처리중인 셀의 좌표
  Coord current_cell;

  // 현재 셀의 오브젝트 ID
  int current_cell_obj;

  // 시야범위 체크 시작지점. 여기서 + max_sight_range*2 까지 체크한다.
  int sight_start_x = camera_pos.x() - max_sight_range;
  int sight_start_y = camera_pos.y() - max_sight_range;

  // 배경을 칠한다.
  DrawPattern(frame_place_offset, frame_place_width, frame_place_height, 0);

  Coord offset;
  // 맵 출력 루프.
  for (int y = -4; y < frame_place_rows+4; ++y) {
    offset.set_y(y);
    current_cell.set_y(yStart + y);
    // 범위초과시 continue
    if (current_cell.y() < 0 || current_cell.y() >= place.GetRows())
      continue;
    for (int x = -4; x < frame_place_cols+4; ++x) {
      offset.set_x(x);
      current_cell.set_x(xStart + x);
      // 범위초과시 continue
      if (current_cell.x() < 0 || current_cell.x() >= place.GetCols())
        continue;
      // 시야선 검사
      if (!(crets[player_id].sight_table()[current_cell.y()-sight_start_y][current_cell.x()-sight_start_x]) &&
          !(current_cell.y() < camera_pos.y()-max_sight_range ||
            current_cell.y() > camera_pos.y()+max_sight_range ||
            current_cell.x() < camera_pos.x()-max_sight_range ||
            current_cell.x() > camera_pos.x()+max_sight_range)) {
        // 지도 출력
        if (place.GetTerrain(current_cell) != -1) {
          DrawSide(place.GetSideTile(current_cell), offset,
            is_y_odd, place.GetHeight(current_cell) - camera_height,
            GetSideLevel(current_cell));
          DrawSide(TileRef(TilesetType::grid, 0x10+grid), offset,
            is_y_odd, place.GetHeight(current_cell) - camera_height,
            GetSideLevel(current_cell));
          if (revealed_place.GetTerrainTypeLiq(current_cell)) {
            DrawFloorLiq(place.GetFloorTile(current_cell), offset,
              is_y_odd, place.GetHeight(current_cell) - camera_height);
          } else {
            DrawFloor(place.GetFloorTile(current_cell), offset,
              is_y_odd, place.GetHeight(current_cell) - camera_height);
          }
          DrawTile(TileRef(TilesetType::grid, grid), offset,
            is_y_odd, place.GetHeight(current_cell) - camera_height);
          // 시야내 오브젝트 출력
          current_cell_obj = place.GetObject(current_cell);
          if (current_cell_obj != -1) {
            DrawTile(object_template[current_cell_obj].tile, offset, is_y_odd, place.GetHeight(current_cell) - camera_height);
          }
          current_cell_obj = place.GetCreatureId(current_cell);
          if (current_cell_obj != -1) {
            DrawTile(crets[current_cell_obj].tile(), offset, is_y_odd, place.GetHeight(current_cell) - camera_height, place.GetSink(current_cell));
            DrawHPGraph(crets[current_cell_obj].tile(), offset, is_y_odd, place.GetHeight(current_cell) - camera_height, place.GetSink(current_cell), crets[current_cell_obj].hp_current(), crets[current_cell_obj].hp_max());
          }
        }
      } else if (revealed_place.GetTerrain(current_cell) != -1) {
        // 밝혀진 지도 출력
        DrawSide(revealed_place.GetSideFogTile(current_cell), offset,
          is_y_odd, revealed_place.GetHeight(current_cell) - camera_height,
          GetSideLevel(current_cell));
        DrawSide(TileRef(TilesetType::grid, 0x10+grid), offset,
          is_y_odd, place.GetHeight(current_cell) - camera_height,
          GetSideLevel(current_cell));
        if (revealed_place.GetTerrainTypeLiq(current_cell)) {
          DrawFloorLiq(revealed_place.GetFloorFogTile(current_cell), offset,
            is_y_odd, revealed_place.GetHeight(current_cell) - camera_height);
        } else {
          DrawFloor(revealed_place.GetFloorFogTile(current_cell), offset,
            is_y_odd, revealed_place.GetHeight(current_cell) - camera_height);
        }
        DrawTile(TileRef(TilesetType::grid, grid), offset,
          is_y_odd, revealed_place.GetHeight(current_cell) - camera_height);
        current_cell_obj = revealed_place.GetObject(current_cell);
        if (current_cell_obj != -1) {
          DrawTile(object_template[current_cell_obj].tile_fog, offset,
          is_y_odd, revealed_place.GetHeight(current_cell) - camera_height);
        }
      }
    }
  }
}

void LocalStage::DrawMinimap() {
  // 기준점의 y축 좌표 가 짝수인지 저장하는 변수.
  // y축이 홀수냐 짝수냐에 따라 x축 헥사맵 출력 방법이 달라져야 함.
  int is_y_odd(camera_pos.y()%2);

  // 지도의 출력 시작 위치. 여기서 + 범위좌표(cols, rows)까지 출력한다.
  int xStart = camera_pos.x() - frame_minimap_cols/2;
  int yStart = camera_pos.y() - frame_minimap_rows/2;

  // 현재 처리중인 셀의 좌표
  Coord current_cell;

  // 현재 셀의 오브젝트 ID
  int current_cell_obj;

  SDL_Rect box;
  box.w = 2;
  box.h = 2;

  // 맵 출력 루프.
  for (int y = 0; y < frame_minimap_rows; ++y) {
    current_cell.set_y(yStart + y);
    // 범위초과시 continue
    if (current_cell.y() < 0 || current_cell.y() >= revealed_place.GetRows())
      continue;
    box.y = frame_minimap_offset.y() + y*2;
    for (int x = -1; x < frame_minimap_cols+1; ++x) {
      current_cell.set_x(xStart + x);
      // 범위초과시 continue
      if (current_cell.x() < 0 || current_cell.x() >= revealed_place.GetCols())
        continue;
      // 밝혀진 지도 출력
      if (revealed_place.GetTerrain(current_cell) != -1) {
        box.x = frame_minimap_offset.x() + x*2 - is_y_odd;
        if (y%2 == is_y_odd)
          ++box.x;
        DrawBox(box, revealed_place.GetTerrainColor(current_cell));
        current_cell_obj = revealed_place.GetObject(current_cell);
        if (current_cell_obj != -1) {
          DrawBox(box, object_template[current_cell_obj].color);
        }
      }
    }
  }
}

void LocalStage::DrawDialogMap() {
  // 기준점의 y축 좌표 가 짝수인지 저장하는 변수.
  // y축이 홀수냐 짝수냐에 따라 x축 헥사맵 출력 방법이 달라져야 함.
  int is_y_odd(1);

  // 지도의 출력 시작 위치. 여기서 + 범위좌표(cols, rows)까지 출력한다.
  int xStart = 0;
  int yStart = 0;

  // 현재 처리중인 셀의 좌표
  Coord current_cell;

  // 현재 셀의 오브젝트 ID
  int current_cell_obj;

  SDL_Rect box;
  box.w = 2;
  box.h = 2;

  // 배경을 칠한다.
  DrawPattern(frame_fullscreen_rect, 0);
  DrawBox(frame_dialog_rect, color_ltgray);
  DrawBox(frame_dialog_title_rect, color_dkgray);
  dialog_title.Clear();
  dialog_title << "탐험 지도 (" << place.GetName() << ")";
  dialog_title.Draw();
  // 맵 출력 루프.
  for (int y = 0; y < frame_dialog_map_rows; ++y) {
    current_cell.set_y(yStart + y);
    // 범위초과시 continue
    if (current_cell.y() < 0 || current_cell.y() >= revealed_place.GetRows())
      continue;
    box.y = frame_dialog_content_rect.y + y*2;
    for (int x = -1; x < frame_dialog_map_cols+1; ++x) {
      current_cell.set_x(xStart + x);
      // 범위초과시 continue
      if (current_cell.x() < 0 || current_cell.x() >= revealed_place.GetCols())
        continue;
      // 밝혀진 지도 출력
      if (revealed_place.GetTerrain(current_cell) != -1) {
        box.x = frame_dialog_content_rect.x + x*2;
        if (y%2 == is_y_odd)
          ++box.x;
        DrawBox(box, revealed_place.GetTerrainColor(current_cell));
        current_cell_obj = revealed_place.GetObject(current_cell);
        if (current_cell_obj != -1) {
          DrawBox(box, object_template[current_cell_obj].color);
        }
      }
    }
  }
}

void LocalStage::Prepare() {
  DrawBackground();
  place.RandomizeTerrain(LandType::island);
  crets.CreateCreature(place, Coord(20, 20), 0); // pc
  SetCameraPos(crets[player_id].pos());
  for (int i = 0; i < 40; ++i)
    crets.CreateCreature(place, Coord(Random()%place.GetCols(), Random()%place.GetRows()), Random()%2+1);
  quit = false;
  map_name = place.GetName();
  player_id = 0;
  PlayMusic(14);
  //TestDialog();
}

void LocalStage::TestStringScanner() {
  StringScanner sc;
  crets[player_id].set_name(sc.Scan(1, true, 60, message, "당신의 이름은 무엇입니까? : "));
  message << "이제부터 " << crets[player_id].name() << "{이} 당신의 이름입니다.\n";
}

void LocalStage::ProcessUserInput() {
  bool end_turn = false;
  SDL_Event event;
  Uint8* key_state = NULL;
  Draw();
  while(!end_turn && !program_quit) {
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        program_quit = true;
      }
      if (event.type == SDL_KEYDOWN) {
        switch( event.key.keysym.sym ) {
        case SDLK_ESCAPE:
          program_quit = true;
          break;

        case SDLK_F1:
          RevealRevealedPlace();
          message << "지도를 밝힙니다.\n";
          Draw();
          break;

        case SDLK_F2:
          ConcealRevealedPlace();
          message << "지도를 숨깁니다.\n";
          Draw();
          break;

        case SDLK_F3:
          TestDialog();
          Draw();
          break;

        case SDLK_F4:
          Draw();
          TestStringScanner();
          Draw();
          break;

        case SDLK_F5:
          message << "현재 시각은 " << GetWorldTimeString() << " 입니다.\n";
          Draw();
          break;

        case SDLK_F6:
          UserShowDialogMap();
          break;

        case SDLK_F7:
          TestEventDialog();
          break;

        case SDLK_1:
          ChangeGrid();
          message << "격자를 " << GetGrid() << "번으로 설정합니다.\n";
          Draw();
          break;

        case SDLK_2:
          place.RandomizeTerrain();
          ConcealRevealedPlace();
          message << "<blue>지형<black>을 다시 생성합니다.\n";
          map_name = place.GetName();
          Draw();
          break;

        case SDLK_3:
          place.RandomizeWall();
          message << "<blue>벽<black>을 다시 생성합니다.\n";
          Draw();
          break;

        case SDLK_4:
          for (int i = 0; i < 10; ++i)
          crets.CreateCreature(place, Coord(Random()%place.GetCols(), Random()%place.GetRows()), Random()%2+1);
          message << "<blue>크리처<black>를 추가 생성합니다. " << "총 크리쳐 : " << crets.GetCount() << " 개\n";
          Draw();
          break;

        case SDLK_5:
            break;

        case SDLK_6:
          place.DrawObjectRectangleA(crets[player_id].pos(), Random()%12+1, Random()%12+1, Random()%0x10);
          UpdateRevealedPlace();
          message << "사각형 벽을 만듭니다 : Type A\n";
          Draw();
          break;

        case SDLK_7:
          place.DrawObjectRectangleB(crets[player_id].pos(), Random()%12+1, Random()%12+1, Random()%0x10);
          UpdateRevealedPlace();
          message << "사각형 벽을 만듭니다 : Type B\n";
          Draw();
          break;

        case SDLK_8:
          place.DrawPool(crets[player_id].pos(), 3, 3);
          UpdateRevealedPlace();
          message << "개울을 만듭니다\n";
          Draw();
          break;

        case SDLK_9:
          message << "<blue><남자><default>가 말했다. “안녕하세요.”\n";
          Draw();
          break;

        case SDLK_0:
          message << "<red><팻말><default>에 쓰여 있기를...\n“공무원의 신분과 정치적 중립성은 법률이 정하는 바에 의하여 보장된다. 대법원장은 국회의 동의를 얻어 대통령이 임명한다. 대한민국은 통일을 지향하며, 자유민주적 기본질서에 입각한 평화적 통일 정책을 수립하고 이를 추진한다.\n국회나 그 위원회의 요구가 있을 때에는 국무총리·국무위원 또는 정부위원은 출석·답변하여야 하며, 국무총리 또는 국무위원이 출석요구를 받은 때에는 국무위원 또는 정부위원으로 하여금 출석·답변하게 할 수 있다.\n모든 국민은 법률이 정하는 바에 의하여 선거권을 가진다. 대통령이 임시회의 집회를 요구할 때에는 기간과 집회요구의 이유를 명시하여야 한다. 모든 국민은 고문을 받지 아니하며, 형사상 자기에게 불리한 진술을 강요당하지 아니한다.\n국회는 선전포고, 국군의 외국에의 파견 또는 외국군대의 대한민국 영역안에서의 주류에 대한 동의권을 가진다. 모든 국민은 법률이 정하는 바에 의하여 국방의 의무를 진다.\n여자의 근로는 특별한 보호를 받으며, 고용·임금 및 근로조건에 있어서 부당한 차별을 받지 아니한다. 헌법재판소의 장은 국회의 동의를 얻어 재판관중에서 대통령이 임명한다.\n모든 국민은 종교의 자유를 가진다. 재판의 심리와 판결은 공개한다. 다만, 심리는 국가의 안전보장 또는 안녕질서를 방해하거나 선량한 풍속을 해할 염려가 있을 때에는 법원의 결정으로 공개하지 아니할 수 있다.\n새로운 회계연도가 개시될 때까지 예산안이 의결되지 못한 때에는 정부는 국회에서 예산안이 의결될 때까지 다음의 목적을 위한 경비는 전년도 예산에 준하여 집행할 수 있다.\n광물 기타 중요한 지하자원·수산자원·수력과 경제상 이용할 수 있는 자연력은 법률이 정하는 바에 의하여 일정한 기간 그 채취·개발 또는 이용을 특허할 수 있다.\n체포·구속·압수 또는 수색을 할 때에는 적법한 절차에 따라 검사의 신청에 의하여 법관이 발부한 영장을 제시하여야 한다. 다만, 현행범인인 경우와 장기 3년 이상의 형에 해당하는 죄를 범하고 도피 또는 증거인멸의 염려가 있을 때에는 사후에 영장을 청구할 수 있다.\n감사원은 원장을 포함한 5인 이상 11인 이하의 감사위원으로 구성한다. 국회는 국가의 예산안을 심의·확정한다. 한 회계연도를 넘어 계속하여 지출할 필요가 있을 때에는 정부는 연한을 정하여 계속비로서 국회의 의결을 얻어야 한다.\n훈장등의 영전은 이를 받은 자에게만 효력이 있고, 어떠한 특권도 이에 따르지 아니한다. 대통령은 제1항과 제2항의 처분 또는 명령을 한 때에는 지체없이 국회에 보고하여 그 승인을 얻어야 한다.\n위원은 정당에 가입하거나 정치에 관여할 수 없다. 국회의원과 정부는 법률안을 제출할 수 있다. 국무위원은 국정에 관하여 대통령을 보좌하며, 국무회의의 구성원으로서 국정을 심의한다.\n통신·방송의 시설기준과 신문의 기능을 보장하기 위하여 필요한 사항은 법률로 정한다. 공무원은 국민전체에 대한 봉사자이며, 국민에 대하여 책임을 진다.\n국가는 여자의 복지와 권익의 향상을 위하여 노력하여야 한다. 제1항의 지시를 받은 당해 행정기관은 이에 응하여야 한다. 대통령후보자가 1인일 때에는 그 득표수가 선거권자 총수의 3분의 1 이상이 아니면 대통령으로 당선될 수 없다.”\n";
          Draw();
          break;

        case SDLK_a:
          place.SetHeight(crets[player_id].pos(), place.GetHeight(crets[player_id].pos())+1);
          UpdateRevealedPlace();
          message << "지형의 높이를 높였습니다. 현재 높이 : " << place.GetHeight(crets[player_id].pos()) << '\n';
          Draw();
          break;

        case SDLK_z:
          place.SetHeight(crets[player_id].pos(), place.GetHeight(crets[player_id].pos())-1);
          UpdateRevealedPlace();
          message << "지형의 높이를 낮췄습니다. 현재 높이 : " << place.GetHeight(crets[player_id].pos()) << '\n';
          Draw();
          break;

        case SDLK_m:
          message << PlayMusicRandom() << "번 음악을 재생합니다.\n";
          Draw();
          break;

        case SDLK_l:
          message << "당신은 지금 " << place.GetTerrainName(crets[player_id].pos()) << " 위에 서 있다.\n";
          Draw();
          break;

        case SDLK_KP_PLUS:
          if (crets[player_id].IncSightRange()) {
              UpdateRevealedPlace();
              message << "시야를 늘렸습니다. 현재 시야 : " << crets[player_id].sight_range() << '\n';
          } else {
              message << "시야를 더 늘릴 수 없습니다.\n";
          }
          Draw();
          break;

        case SDLK_KP_MINUS:
          if (crets[player_id].DecSightRange()) {
              UpdateRevealedPlace();
              message << "시야를 줄였습니다. 현재 시야 : "
              << crets[player_id].sight_range() << "\n";
          } else {
              message << "시야를 더 줄일 수 없습니다.\n";
          }
          Draw();
          break;

        case SDLK_LEFT:
          key_state = SDL_GetKeyState(NULL);
          if (key_state[SDLK_UP]) {
            PlayerActMove(HexDir::nwest);
            end_turn = true;
          } else if (key_state[SDLK_DOWN]) {
            PlayerActMove(HexDir::swest);
            end_turn = true;
          } else {
            PlayerActMove(HexDir::west);
            end_turn = true;
          }
          break;

        case SDLK_RIGHT:
          key_state = SDL_GetKeyState(NULL);
          if (key_state[SDLK_UP]) {
            PlayerActMove(HexDir::neast);
            end_turn = true;
          } else if (key_state[SDLK_DOWN]) {
            PlayerActMove(HexDir::seast);
            end_turn = true;
          } else {
            PlayerActMove(HexDir::east);
            end_turn = true;
          }
          break;

        case SDLK_KP4:
          PlayerActMove(HexDir::west);
          end_turn = true;
          break;

        case SDLK_KP6:
          PlayerActMove(HexDir::east);
          end_turn = true;
          break;

        case SDLK_KP7:
        case SDLK_HOME:
          PlayerActMove(HexDir::nwest);
          end_turn = true;
          break;

        case SDLK_KP9:
        case SDLK_PAGEUP:
          PlayerActMove(HexDir::neast);
          end_turn = true;
          break;

        case SDLK_KP1:
        case SDLK_END:
          PlayerActMove(HexDir::swest);
          end_turn = true;
          break;

        case SDLK_KP3:
        case SDLK_PAGEDOWN:
          PlayerActMove(HexDir::seast);
          end_turn = true;
          break;

        case SDLK_KP5:
          PlayerActWait();
          end_turn = true;
          break;

        case SDLK_KP_DIVIDE:
          if(music_number > 0) {
            --music_number;
          } else {
            music_number = number_of_musics - 1;
          }
          PlayMusic(music_number);
          message << music_number << "번 음악을 재생합니다.\n";
          Draw();
          break;

        case SDLK_KP_MULTIPLY:
          if(music_number < number_of_musics - 1) {
            ++music_number;
          } else {
            music_number = 0;
          }
          PlayMusic(music_number);
          message << music_number << "번 음악을 재생합니다.\n";
          Draw();
          break;

        default:
          break;
        }
      }
    }
  }
}

void LocalStage::FlowTick() {
  world_tick++;
  for (int id = 0; id<crets.GetCount(); ++id) {
    crets[id].FlowTick();
  }
}

void LocalStage::ProcessCreatures() {
  for (int id = 0; id<crets.GetCount(); ++id) {
    if(crets[id].is_removed())
      continue;
    // AI 처리범위 밖이면 8분의 1 확률로 행동, 최대치 밖이면 AI 행동 않음
    if (crets[id].IsActiveTurn()) {
      if ((crets[id].GetDistanceWith(player_id) < ai_distance_limit) &&
          (crets[id].GetDistanceWith(player_id) < ai_distance_slow ||
           Random()%8 == 0)) {
        // 행동 전 시야 업데이트
        crets[id].UpdateSightTable();
        if (id == player_id) {
          // 플레이어 행동
          ProcessUserInput();
        } else {
          // NPC 행동
          ProcessCreatureAct(id);
        }
        // 행동 후 시야 업데이트
        crets[id].UpdateSightTable();
      }
      crets[id].ActWait();
    }
  }
}

void LocalStage::ProcessCreatureAct(const int& id) {
  crets[id].ActWalkTo((HexDir)(Random()%6));
}

void LocalStage::SetCameraPosToPlayer() {
  SetCameraPos(crets[player_id].pos());
}

void LocalStage::PlayerActMove(const HexDir& dir) {
  HexPos target_pos(crets[player_id].pos());
  target_pos.MoveTo(dir);
  if (place.IsOutOfRange(target_pos)) {
    // 범위 초과
    ;
  } else if (place.HasCreature(target_pos)) {
    // 기본 공격
    crets[player_id].ActBaseAttack(target_pos);
  } else {
    // 이동
    crets[player_id].ActWalkTo(dir);
    SetCameraPosToPlayer();
  }
}

void LocalStage::PlayerActWait() {
  crets[player_id].ActWait();
}

void LocalStage::UserShowDialogMap() {
  DrawDialogMap();
  Refresh();
  WaitForInputIdle();
  Draw();
}

void LocalStage::TestEventDialog() {
  DrawPattern(frame_fullscreen_rect, 0);
  DrawBox(frame_dialog_rect, color_purple);
  DrawImage(frame_dialog_illust_rect, 3);
  dialog_script.Clear();
  dialog_script << " 오늘은 위대한 혁명의 날.\n 많은 이들이 궁으로 들어와, 례법의 의식을 거행한다.";
  dialog_script.Draw();
  Refresh();
  WaitForInputIdle();
  Draw();
}

void LocalStage::UpdateStatusFrame() {
  status.Clear();
  status << "<" << crets[player_id].name() << "> "
         << "생명: " << crets[player_id].hp_current() << "/"
         << crets[player_id].hp_max() << " "
         << "정상";
}

void LocalStage::Draw() {
  crets[player_id].UpdateSightTable();
  UpdateRevealedPlace();
  DrawPlace();
  DrawPattern(frame_info_rect, 1);
  DrawMinimap();
  map_name.Draw();
  UpdateStatusFrame();
  DrawPlayerHPGraph(crets[player_id].hp_current(), crets[player_id].hp_max());
  status.Draw();
  DrawBox(frame_message_rect, color_ltgray);
  message.Draw();
  message.Clear();
  Refresh();
}

void LocalStage::Run() {
  while(1) {
    if(quit)
      return;
    if(program_quit)
      return;
    ProcessCreatures();
    FlowTick();
  }
}
