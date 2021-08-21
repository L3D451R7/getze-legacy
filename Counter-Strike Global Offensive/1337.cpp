int __usercall Autowall::FireBullets@<eax>(float *a1@<edx>, int a2@<ecx>, int a3@<ebp>, int a4@<edi>, int a5@<esi>, Vector *direction, int a7, int a8)
{
  // [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]

  v54 = a3;
  v55 = retaddr;
  v43 = a2;
  sub_6AC63991(a8);
  v52 = (v9 + 44);
  (*(*a7 + 8))(a7, v43, a4, a5, &v56);
  (*(*a7 + 16))(a7, 0);
  start.x = *a1++;
  v10 = 0.0;
  m_trace_length = 0.0;
  start.y = *a1;
  start.z = a1[1];
  m_filter = a7;
  v12 = a8;
  v13 = *(a7 + 28) - 5.0;
  v51 = *(a8 + 32);
  v44 = fmaxf(1.0, v13);
  if ( v51 > v44 )
  {
    v37 = qword_5D912B98;
    while ( 1 )
    {
      distance = v12->m_max_length - v10;
      dirx = direction->x;
      diry = direction->y;
      dirz = direction->z;
      v36.m_pWorldAxisTransform = 0;
      v36.m_IsRay = 1;
      v50 = start.x;
      v36.m_Start.x = start.x;
      lenght_delta = distance;
      *&v46 = (dirx * distance) + start.x;
      v49 = start.y;
      v47 = (diry * distance) + start.y;
      v53 = start.z;
      v42 = (dirz * distance) + start.z;
      v36.m_Delta.x = *&v46 - start.x;
      v36.m_Delta.y = v47 - start.y;
      v36.m_Delta.z = v42 - start.z;
      v36.m_Start.y = start.y;
      v36.m_Start.z = start.z;
      v36.m_StartOffset = 0i64;
      v36.m_Extents = 0i64;
      mask = *(m_filter + 24);
      v36.m_IsSwept = (((v36.m_Delta.y * v36.m_Delta.y) + (v36.m_Delta.x * v36.m_Delta.x)) + ((v42 - v53) * (v42 - v53))) != 0.0;
      (*(*EngineTrace + 20))(EngineTrace, &v36, mask, m_filter, &v12->enter_trace);
      if ( (*(m_filter + 24) & 0x40000000) != 0 )// if (autowall_mask & hitbox)
      {
        v18 = v12->enter_trace.endpos.x;
        v19 = v12->enter_trace.endpos.z - v53;
        v20 = (direction->y * 40.0) + v47;
        v21 = (direction->z * 40.0) + v42;
        v22 = v12->enter_trace.endpos.y - v49;
        v38.x = (direction->x * 40.0) + *&v46;
        v38.y = v20;
        v38.z = v21;
        ClipTraceToPlayerOrPlayers(
          &v38.x,
          &start.x,
          fsqrt((((v18 - v50) * (v18 - v50)) + (v22 * v22)) + (v19 * v19))
        / fsqrt((((v50 - v38.x) * (v50 - v38.x)) + ((v49 - v20) * (v49 - v20))) + ((v53 - v21) * (v53 - v21))),
          *(m_filter + 24),
          m_filter,
          &v12->enter_trace);
      }
      if ( v12->enter_trace.fraction == 1.0 )
        break;
      v23 = &v12->enter_trace.endpos;
      v24 = (a8 + 12 * (*(a8 + 128) + 11));
      *v24 = LODWORD(v23->x);
      v23 = (v23 + 4);
      *++v24 = LODWORD(v23->x);
      v24[1] = LODWORD(v23->y);
      ++*(a8 + 128);
      v25 = v52;
      v47 = COERCE_FLOAT((*(*m_PhysProps + 20))(v52->surface.surfaceProps));
      v53 = *&v25->m_pEnt;
      v49 = v53;
      if ( v53 == 0.0 // if (!hit_player || !hit_player->isPlayer()
        || (v50 = 2.6253989e-30, v41 = CallVirtualFunction(LODWORD(v49), 240451431), v26 = v53, !v41(LODWORD(v53))) )
      {
        v26 = 0.0;
        v53 = 0.0; // hit_player = nullptr;
      }
      m_filter = a7;
      (*(*a7 + 16))(a7, COERCE_FLOAT(LODWORD(v26)));
      v12 = a8;
      m_trace_length = (lenght_delta * v52->fraction) + m_trace_length;
      v27 = *(a8 + 24);
      sub_6AD1FAA9();
      v51 = v27 * v51;
      *&v46 = v51;
      if ( v44 > v51 )
        break;
      if ( m_trace_length > 3000.0 && *(a8 + 16) > 0.0 || *&v37 > *(LODWORD(v47) + 88) )
        *(a8 + 40) = 0; //can_penetrate = false;
      v28 = v52;
      if ( (*(a7 + 24) & 0x40000000) == 0 )// if !(autowall_mask & hitbox)
        v52->hitgroup = 1;
      v29 = v28->m_pEnt;
      v50 = *&v29;
      if ( *&v29 != 0.0 ) //hit_player != null
      {
        lenght_delta = *&v29;
        v49 = 2.6253985e-30;
        v40 = CallVirtualFunction(v29, 0xE54FF65);
        if ( v40(LODWORD(v50)) ) // hit_player->isPlayer()
        {
          v30 = LODWORD(v53);
          if ( v53 == 0.0 )
            goto LABEL_34;
          if ( (v52->hitgroup - 1) <= 7 && !*((m_bIsPlayerGhost ^ *dword_5D9188F0) + LODWORD(v53)) )
          {
            if ( byte_5D91DF18[32 * (*(*(LODWORD(v53) + 8) + 40))(LODWORD(v53) + 8)] || v43 && !IsEnemy(v43, v30) ) //HasArmor inlined
            {
LABEL_34:
              v12 = a8;
              break;
            }
            if ( (*(a7 + 24) & 0x40000000) != 0 ) // if (autowall_mask & hitbox)
            {
              v31 = ScaleDamage(*(a8 + 112), v30, *(a8 + 28), *(a8 + 12));
              v32 = *&v46 * v31;
            }
            else
            {
              v32 = *&v46;
            }
            result = v32;
            *(a8 + 8) = v32;
            return result;
          }
        }
        m_filter = a7;
      }
      v12 = a8;
      if ( *(a8 + 40) <= 0 ) //if !can_penetrate
        break;
      if ( Autowall::HandleBulletsPenetration(&direction->x, &start, a8, m_filter, LODWORD(v47), (a8 + 40), &v51) )
      {
        *(a8 + 36) = 1;
        break;
      }
      if ( v51 <= v44 )
        break;
      v10 = m_trace_length;
    }
  }
  v12->dword8 = 0;
  return 0;
}

char __fastcall Autowall::HandleBulletsPenetration(float *vec_start, _DWORD *startpos, struct_a3 *a3, int a4, surfacedata_t *enter_surf, int *penetration_count, float *m_damage_ptr)
{
  // [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]

  enter_trace = &a3->m_enter_trace;
  enter_material = enter_surf->game.material;
  isSolidSurf = (a3->m_enter_trace.contents >> 3) & 0xFFFFFF01;
  isSolidSurf2 = isSolidSurf;
  isLightSurf = LOBYTE(a3->m_enter_trace.surface.flags) >> 7;
  if ( (*penetration_count
     || isSolidSurf
     || SLOBYTE(a3->m_enter_trace.surface.flags) < 0
     || enter_material == CHAR_TEX_GLASS
     || enter_material == CHAR_TEX_GRATE)
    && a3->m_penetration_power > 0.0
    && *penetration_count > 0 )
  {
    a4a = 90.0;
    v10 = *(a4 + 28);
    if...
    exit_trace.hitgroup = -1;
    exit_trace.hitbox = -1;
    *&exit_trace.allSolid = 0;
    exit_trace.m_pEnt = 0;
    exit_trace.fraction = 1.0;
    MEMORY[0x773094D0](&exit_trace, 0, 84);
    if ( !Autowall::TraceToExit(enter_trace, vec_start, 4.0, a4a, a4, &exit_trace) )
    {
      v15 = GetPointContents;
      if ( (*(*a4 + 4))(a4) == 1 ) // mega optimization
        v15 = GetPointContents_WorldOnly;
      if ( (v15(&a3->m_enter_trace.endpos, 0x600400B, 0) & 0x600400B) == 0 )
        return 1;
    }
    exit_surf = (*(*m_PhysProps + 20))(m_PhysProps, exit_trace.surface.surfaceProps);
    exit_material = exit_surf->game.material;
    if ( enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS )
    {
      combined_damage_modifier = 0.050000001;
      combined_penetration_modifier = 3.0;
    }
    else if ( isSolidSurf2 || isLightSurf )
    {
      combined_damage_modifier = 0.16;
      combined_penetration_modifier = 1.0;
    }
    else
    {
      combined_damage_modifier = 0.16;
      combined_penetration_modifier = (exit_surf->game.penetrationmodifier + enter_surf->game.penetrationmodifier) * 0.5;
    }
    if ( exit_material == enter_material )
    {
      if ( exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD )
      {
        combined_penetration_modifier = 3.0;
      }
      else if ( exit_material == CHAR_TEX_PLASTIC )
      {
        combined_penetration_modifier = 2.0;
      }
    }
    thickness = fsqrt(
                  (((exit_trace.endpos.x - enter_trace->endpos.x) * (exit_trace.endpos.x - enter_trace->endpos.x))
                 + ((exit_trace.endpos.y - enter_trace->endpos.y) * (exit_trace.endpos.y - enter_trace->endpos.y)))
                + ((exit_trace.endpos.z - enter_trace->endpos.z) * (exit_trace.endpos.z - enter_trace->endpos.z)));
    modifier = fmaxf(0.0, 1.0 / combined_penetration_modifier);
    m_damage = *m_damage_ptr;
    penetration_mod = fmaxf(0.0, 3.0 / a3->m_penetration_power * 1.25) * (modifier * 3.0);
    lost_damage = fmaxf(
                    0.0,
                    (penetration_mod + (m_damage * combined_damage_modifier))
                  + (((thickness * thickness) * modifier) / 24.0));
    if ( (LODWORD(lost_damage) & 0x7F800000) != 0x7F800000 ) // checkaet chto ne INF/NAN (std::isfinite mb?)
    {
      v26 = m_damage - lost_damage;
      *new_damage = v26;
      if ( v26 >= 1.0 )
      {
        *startpos = exit_trace.endpos;
        --*penetration_count;
        return 0;
      }
    }
    *new_damage = 0.0;
  }
  return 1;
}

int __userpurge Autowall::TraceToExit@<eax>(trace_t *enter_trace@<edx>, Vector *dir@<ecx>, float step_1@<xmm0>, float a4@<xmm1>, int *a5, trace_t *exit_trace)
{
  // [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]

  v34 = 0;
  v36 = a4;
  v6 = *a5;
  step = step_1;
  v8 = (*(v6 + 4))(a5);
  v9 = GetPointContents; // mega optimization
  if ( v8 == 1 )
    v9 = GetPointContents_WorldOnly;
  v37 = v9;
  v10 = 0;
  if ( a4 >= 0.0 )
  {
    fDistance = 0.0;
    dir_1 = dir;
    while ( 1 )
    {
      v30 = fDistance + step;
      v31 = (dir_1->x * (fDistance + step)) + enter_trace->endpos.x;
      v32 = (dir_1->y * (fDistance + step)) + enter_trace->endpos.y;
      v13 = a5[6];
      v33 = (dir_1->z * (fDistance + step)) + enter_trace->endpos.z;
      v14 = v37(&v31, v13, 0);
      v15 = v34;
      if ( !v34 )
        v15 = v14;
      v34 = v15;
      if ( (v14 & 0x600400B) == 0 || (v14 & 0x40000000) != 0 && v14 != v15 )
      {
        v16 = dir->x * step;
        LOWORD(v46) = 257;
        *v39 = v31;
        *&v39[1] = v32;
        v17 = v31 - v16;
        *&v39[2] = v33;
        v18 = v32 - (dir->y * step);
        v19 = dir->z * step;
        v45 = 0;
        v40 = v17 - v31;
        *&v43[1] = 0i64;
        v44 = 0i64;
        v41 = v18 - v32;
        v42 = (v33 - v19) - v33;
        if ( (((v41 * v41) + (v40 * v40)) + (v42 * v42)) == 0.0 )
          BYTE1(v46) = 0;
        (*(*EngineTrace + 20))(EngineTrace, v39, a5[6], a5, exit_trace);
        v20 = exit_trace->startSolid;
        if ( v20 && (exit_trace->surface.flags & SURF_HITBOX) != 0 )
        {
          v21 = enter_trace->endpos.x;
          v22 = enter_trace->endpos.y;
          v23 = exit_trace->m_pEnt;
          v47.m_Start.x = v31;
          v24 = enter_trace->endpos.z;
          v47.m_Delta.x = v21 - v31;
          v47.m_Delta.y = v22 - v32;
          filter[1] = v23;
          filter[0] = CTraceFilter::VTable;
          v47.m_Delta.z = v24 - v33;
          filter[2] = 0;
          v48.m_Start.y = v32;
          filter[3] = 0;
          v48.m_Start.z = v33;
          v48.m_StartOffset = 0i64;
          v48.m_Extents = 0i64;
          v48.m_pWorldAxisTransform = 0;
          v48.m_IsRay = 1;
          v48.m_IsSwept = (((v48.m_Delta.y * v48.m_Delta.y) + (v48.m_Delta.x * v48.m_Delta.x))
                         + (v48.m_Delta.z * v48.m_Delta.z)) != 0.0;
          (*(*EngineTrace + 20))(EngineTrace, &v48, 0x600400B, filter, exit_trace);
          if ( (exit_trace->fraction < 1.0 || exit_trace->allSolid) && !exit_trace->startSolid )
            return 1;
        }
        else if ( exit_trace->fraction >= 1.0 && !exit_trace->allSolid || v20 )
        {
          if ( enter_trace->m_pEnt && !IsWorldEntity(enter_trace) && IsBreakable(enter_trace->m_pEnt) )
          {
            exit_trace->surface.surfaceProps = enter_trace->surface.surfaceProps;
            v26 = dir->y + enter_trace->endpos.y;
            v27 = dir->z + enter_trace->endpos.z;
            exit_trace->endpos.x = dir->x + enter_trace->endpos.x;
            exit_trace->endpos.y = v26;
            exit_trace->endpos.z = v27;
            return 1;
          }
        }
        else
        {
          v29 = LOBYTE(enter_trace->surface.flags) >> 7;
          if ( SLOBYTE(exit_trace->surface.flags) >= 0 )
            goto LABEL_25;
          if ( IsBreakable(exit_trace->m_pEnt) && IsBreakable(enter_trace->m_pEnt) )
            return 1;
          if ( v29 )
          {
LABEL_25:
            dir_1 = dir;
            if ( (((exit_trace->plane.normal.y * dir->y) + (dir->x * exit_trace->plane.normal.x))
                + (exit_trace->plane.normal.z * dir->z)) <= 1.0 )
              return 1;
            goto LABEL_31;
          }
        }
      }
      dir_1 = dir;
LABEL_31:
      fDistance = v30;
      if ( v36 < v30 )
        return 0;
    }
  }
  return v10;
}