/* ============================================================
   智慧社区管理平台 - 数据库初始化脚本 (SQLite)
   ============================================================ */

-- ========== 1. SYS 系统管理域 ==========

CREATE TABLE IF NOT EXISTS sys_user (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        VARCHAR(64)  NOT NULL UNIQUE,
    password        VARCHAR(128) NOT NULL,
    nickname        VARCHAR(64),
    real_name       VARCHAR(64),
    avatar          VARCHAR(256),
    phone           VARCHAR(20),
    email           VARCHAR(128),
    id_card         VARCHAR(64),
    gender          INTEGER DEFAULT 0,
    birthday        DATE,
    status          INTEGER NOT NULL DEFAULT 0,
    user_type       INTEGER NOT NULL DEFAULT 0,
    last_login_time DATETIME,
    last_login_ip   VARCHAR(64),
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0,
    remark          VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS sys_role (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    role_name   VARCHAR(64)  NOT NULL,
    role_key    VARCHAR(64)  NOT NULL UNIQUE,
    role_domain VARCHAR(32)  NOT NULL,
    data_scope  INTEGER NOT NULL DEFAULT 6,
    sort_order  INTEGER DEFAULT 0,
    status      INTEGER DEFAULT 0,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag    INTEGER DEFAULT 0,
    remark      VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS sys_user_role (
    user_id INTEGER NOT NULL,
    role_id INTEGER NOT NULL,
    PRIMARY KEY (user_id, role_id)
);

CREATE TABLE IF NOT EXISTS sys_menu (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    parent_id   INTEGER DEFAULT 0,
    menu_name   VARCHAR(64)  NOT NULL,
    menu_type   INTEGER NOT NULL,
    path        VARCHAR(256),
    component   VARCHAR(256),
    permission  VARCHAR(128),
    icon        VARCHAR(64),
    sort_order  INTEGER DEFAULT 0,
    visible     INTEGER DEFAULT 1,
    status      INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sys_role_menu (
    role_id INTEGER NOT NULL,
    menu_id INTEGER NOT NULL,
    PRIMARY KEY (role_id, menu_id)
);

CREATE TABLE IF NOT EXISTS sys_org (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    parent_id  INTEGER DEFAULT 0,
    org_name   VARCHAR(128) NOT NULL,
    org_type   INTEGER NOT NULL,
    org_code   VARCHAR(64) UNIQUE,
    leader     VARCHAR(64),
    phone      VARCHAR(20),
    address    VARCHAR(256),
    ancestors  VARCHAR(1024),
    sort_order INTEGER DEFAULT 0,
    status     INTEGER DEFAULT 0,
    create_by  INTEGER,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag   INTEGER DEFAULT 0,
    remark     VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS sys_user_org (
    user_id    INTEGER NOT NULL,
    org_id     INTEGER NOT NULL,
    is_primary INTEGER DEFAULT 0,
    PRIMARY KEY (user_id, org_id)
);

CREATE TABLE IF NOT EXISTS sys_dict_type (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    dict_name VARCHAR(128) NOT NULL,
    dict_type VARCHAR(128) NOT NULL UNIQUE,
    status    INTEGER DEFAULT 0,
    remark    VARCHAR(500),
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS sys_dict_data (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    dict_type   VARCHAR(128) NOT NULL,
    dict_label  VARCHAR(128) NOT NULL,
    dict_value  VARCHAR(128) NOT NULL,
    sort_order  INTEGER DEFAULT 0,
    css_class   VARCHAR(64),
    status      INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sys_config (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    config_name  VARCHAR(128) NOT NULL,
    config_key   VARCHAR(128) NOT NULL UNIQUE,
    config_value VARCHAR(512),
    config_type  INTEGER DEFAULT 0,
    remark       VARCHAR(500),
    create_time  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS sys_operation_log (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER,
    username        VARCHAR(64),
    module          VARCHAR(64),
    operation       VARCHAR(128),
    method          VARCHAR(256),
    request_url     VARCHAR(256),
    request_method  VARCHAR(16),
    request_params  TEXT,
    response_result TEXT,
    ip              VARCHAR(64),
    status          INTEGER DEFAULT 0,
    error_msg       TEXT,
    cost_time       INTEGER DEFAULT 0,
    operation_time  DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS sys_login_log (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id   INTEGER,
    username  VARCHAR(64),
    ip        VARCHAR(64),
    location  VARCHAR(256),
    browser   VARCHAR(128),
    os        VARCHAR(128),
    status    INTEGER DEFAULT 0,
    message   VARCHAR(256),
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS sys_file (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    file_name   VARCHAR(256),
    file_path   VARCHAR(512),
    file_size   INTEGER,
    file_type   VARCHAR(64),
    file_ext    VARCHAR(16),
    module      VARCHAR(64),
    biz_id      INTEGER,
    upload_by   INTEGER,
    upload_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ========== 2. CM 基础档案域 ==========

CREATE TABLE IF NOT EXISTS cm_estate (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    org_id              INTEGER,
    property_company_id INTEGER,
    estate_name         VARCHAR(128) NOT NULL,
    estate_code         VARCHAR(64) UNIQUE,
    address             VARCHAR(256),
    total_area          REAL,
    total_buildings     INTEGER,
    total_houses        INTEGER,
    built_year          INTEGER,
    green_rate          REAL,
    parking_spaces      INTEGER,
    longitude           REAL,
    latitude            REAL,
    status              INTEGER DEFAULT 0,
    create_by           INTEGER,
    create_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by           INTEGER,
    update_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag            INTEGER DEFAULT 0,
    remark              VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_building (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id      INTEGER NOT NULL,
    building_name  VARCHAR(64),
    building_code  VARCHAR(32),
    total_units    INTEGER,
    total_floors   INTEGER,
    has_elevator   INTEGER DEFAULT 0,
    elevator_count INTEGER DEFAULT 0,
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0,
    remark         VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_unit (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    building_id INTEGER NOT NULL,
    unit_name  VARCHAR(32),
    unit_code  VARCHAR(32),
    create_by  INTEGER,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by  INTEGER,
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag   INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_house (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    unit_id       INTEGER,
    estate_id     INTEGER,
    house_code    VARCHAR(32) UNIQUE,
    floor         INTEGER,
    room_number   VARCHAR(16),
    area          REAL,
    house_type    VARCHAR(32),
    house_status  INTEGER DEFAULT 0,
    owner_name    VARCHAR(64),
    owner_phone   VARCHAR(64),
    create_by     INTEGER,
    create_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by     INTEGER,
    update_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag      INTEGER DEFAULT 0,
    remark        VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_resident (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id          INTEGER,
    name             VARCHAR(64) NOT NULL,
    id_card          VARCHAR(64),
    gender           INTEGER DEFAULT 0,
    phone            VARCHAR(64),
    phone_display    VARCHAR(20),
    birthday         DATE,
    photo            VARCHAR(256),
    nationality      VARCHAR(32),
    education        VARCHAR(32),
    occupation       VARCHAR(64),
    political_status VARCHAR(16),
    is_special       INTEGER DEFAULT 0,
    special_type     VARCHAR(64),
    emergency_contact VARCHAR(64),
    emergency_phone  VARCHAR(64),
    status           INTEGER DEFAULT 0,
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0,
    remark           VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_family (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    family_name     VARCHAR(64),
    house_id        INTEGER,
    head_resident_id INTEGER,
    member_count    INTEGER DEFAULT 1,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_family_member (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    family_id   INTEGER NOT NULL,
    resident_id INTEGER NOT NULL,
    relation    VARCHAR(32),
    is_head     INTEGER DEFAULT 0,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS cm_house_resident (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    house_id      INTEGER NOT NULL,
    resident_id   INTEGER NOT NULL,
    relation_type INTEGER NOT NULL DEFAULT 1,
    start_date    DATE,
    end_date      DATE,
    status        INTEGER DEFAULT 0,
    create_time   DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS cm_vehicle (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    plate_number    VARCHAR(16) UNIQUE,
    vehicle_brand   VARCHAR(64),
    vehicle_color   VARCHAR(32),
    vehicle_type    INTEGER DEFAULT 1,
    estate_id       INTEGER,
    parking_space_id INTEGER,
    status          INTEGER DEFAULT 0,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0,
    remark          VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_vehicle_owner (
    vehicle_id  INTEGER NOT NULL,
    resident_id INTEGER NOT NULL,
    relation    VARCHAR(32) DEFAULT '车主',
    PRIMARY KEY (vehicle_id, resident_id)
);

CREATE TABLE IF NOT EXISTS cm_parking_space (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id   INTEGER NOT NULL,
    space_code  VARCHAR(32),
    area_name   VARCHAR(64),
    space_type  INTEGER DEFAULT 1,
    status      INTEGER DEFAULT 0,
    create_by   INTEGER,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by   INTEGER,
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag    INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_facility (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id        INTEGER,
    facility_name    VARCHAR(128),
    facility_type    INTEGER,
    facility_code    VARCHAR(64),
    location         VARCHAR(256),
    install_date     DATE,
    warranty_expire  DATE,
    last_maintenance DATE,
    next_maintenance DATE,
    status           INTEGER DEFAULT 0,
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0,
    remark           VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS cm_grid (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    community_org_id INTEGER NOT NULL,
    grid_name        VARCHAR(64),
    grid_code        VARCHAR(32),
    description      VARCHAR(256),
    grid_worker_id   INTEGER,
    cover_estates    VARCHAR(512),
    longitude        REAL,
    latitude         REAL,
    status           INTEGER DEFAULT 0,
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_property_company (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    org_id          INTEGER,
    company_name    VARCHAR(128) NOT NULL,
    legal_person    VARCHAR(64),
    license_no      VARCHAR(64),
    service_scope   VARCHAR(256),
    contact_phone   VARCHAR(20),
    contract_start  DATE,
    contract_end    DATE,
    status          INTEGER DEFAULT 0,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_owner_committee (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    org_id          INTEGER,
    estate_id       INTEGER NOT NULL,
    committee_name  VARCHAR(128),
    established_date DATE,
    term_start      DATE,
    term_end        DATE,
    member_count    INTEGER DEFAULT 0,
    status          INTEGER DEFAULT 0,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_owner_committee_member (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    committee_id  INTEGER NOT NULL,
    user_id       INTEGER NOT NULL,
    position      VARCHAR(32),
    join_date     DATE,
    create_time   DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS cm_special_group (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    resident_id     INTEGER NOT NULL,
    group_type      INTEGER,
    detail          VARCHAR(256),
    care_level      INTEGER DEFAULT 1,
    care_frequency  VARCHAR(32),
    care_worker_id  INTEGER,
    last_visit_time DATETIME,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_social_org (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    org_id          INTEGER,
    org_name        VARCHAR(128),
    org_category    INTEGER,
    contact_person  VARCHAR(64),
    contact_phone   VARCHAR(20),
    service_scope   VARCHAR(256),
    status          INTEGER DEFAULT 0,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

-- ========== 3. WO 工单域 ==========

CREATE TABLE IF NOT EXISTS wo_work_order (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    order_no            VARCHAR(32) UNIQUE,
    estate_id           INTEGER,
    house_id            INTEGER,
    reporter_id         INTEGER NOT NULL,
    reporter_name       VARCHAR(64),
    reporter_phone      VARCHAR(20),
    order_type          INTEGER NOT NULL,
    priority            INTEGER DEFAULT 1,
    title               VARCHAR(256) NOT NULL,
    description         TEXT,
    location_desc       VARCHAR(256),
    images              VARCHAR(1024),
    status              INTEGER NOT NULL DEFAULT 0,
    accept_by           INTEGER,
    accept_time         DATETIME,
    assign_to           INTEGER,
    assign_time         DATETIME,
    finish_time         DATETIME,
    close_time          DATETIME,
    result_desc         TEXT,
    result_images       VARCHAR(1024),
    sla_deadline        DATETIME,
    is_escalated        INTEGER DEFAULT 0,
    escalate_to_event_id INTEGER,
    source              INTEGER DEFAULT 0,
    create_by           INTEGER,
    create_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by           INTEGER,
    update_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag            INTEGER DEFAULT 0,
    remark              VARCHAR(500)
);

-- ========== 4. GE 社区治理域 ==========

CREATE TABLE IF NOT EXISTS ge_event (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    event_no            VARCHAR(32) UNIQUE,
    grid_id             INTEGER,
    community_org_id    INTEGER,
    reporter_id         INTEGER NOT NULL,
    reporter_name       VARCHAR(64),
    event_category      INTEGER NOT NULL,
    priority            INTEGER DEFAULT 1,
    title               VARCHAR(256) NOT NULL,
    description         TEXT,
    location            VARCHAR(256),
    longitude           REAL,
    latitude            REAL,
    images              VARCHAR(1024),
    status              INTEGER NOT NULL DEFAULT 0,
    reviewer_id         INTEGER,
    review_time         DATETIME,
    assign_to           INTEGER,
    assign_org_id       INTEGER,
    assign_time         DATETIME,
    finish_time         DATETIME,
    archive_time        DATETIME,
    result_desc         TEXT,
    result_images       VARCHAR(1024),
    sla_deadline        DATETIME,
    is_supervised       INTEGER DEFAULT 0,
    is_coordinated      INTEGER DEFAULT 0,
    source              INTEGER DEFAULT 0,
    create_by           INTEGER,
    create_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by           INTEGER,
    update_time         DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag            INTEGER DEFAULT 0,
    remark              VARCHAR(500)
);

CREATE TABLE IF NOT EXISTS ge_event_flow (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    event_id      INTEGER NOT NULL,
    action        VARCHAR(64),
    operator_id   INTEGER,
    operator_name VARCHAR(64),
    from_status   INTEGER,
    to_status     INTEGER,
    comment       TEXT,
    images        VARCHAR(1024),
    action_time   DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS ge_inspection (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    grid_id      INTEGER,
    inspector_id INTEGER NOT NULL,
    plan_id      INTEGER,
    start_time   DATETIME,
    end_time     DATETIME,
    duration     INTEGER,
    route_points TEXT,
    issue_count  INTEGER DEFAULT 0,
    summary      TEXT,
    status       INTEGER DEFAULT 0,
    create_by    INTEGER,
    create_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by    INTEGER,
    update_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag     INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS ge_inspection_plan (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    grid_id      INTEGER,
    plan_name    VARCHAR(128),
    frequency    VARCHAR(32),
    inspector_id INTEGER,
    start_date   DATE,
    end_date     DATE,
    status       INTEGER DEFAULT 0,
    create_by    INTEGER,
    create_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by    INTEGER,
    update_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag     INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS ge_visit_record (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    special_group_id INTEGER,
    resident_id      INTEGER,
    visitor_id       INTEGER NOT NULL,
    visit_time       DATETIME,
    visit_type       INTEGER DEFAULT 1,
    content          TEXT,
    images           VARCHAR(1024),
    found_issues     VARCHAR(512),
    follow_up        VARCHAR(256),
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS ge_supervision (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    event_id     INTEGER NOT NULL,
    supervisor_id INTEGER NOT NULL,
    supervise_to INTEGER,
    deadline     DATETIME,
    reason       VARCHAR(256),
    status       INTEGER DEFAULT 0,
    feedback     TEXT,
    feedback_time DATETIME,
    create_by    INTEGER,
    create_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by    INTEGER,
    update_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag     INTEGER DEFAULT 0
);

-- ========== 5. SV 社区服务域 ==========

CREATE TABLE IF NOT EXISTS sv_volunteer (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id        INTEGER NOT NULL,
    resident_id    INTEGER,
    skills         VARCHAR(256),
    available_time VARCHAR(128),
    total_hours    REAL DEFAULT 0,
    status         INTEGER DEFAULT 0,
    register_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sv_volunteer_activity (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    community_org_id INTEGER,
    title            VARCHAR(256) NOT NULL,
    description      TEXT,
    activity_type    INTEGER,
    location         VARCHAR(256),
    start_time       DATETIME,
    end_time         DATETIME,
    need_count       INTEGER,
    enrolled_count   INTEGER DEFAULT 0,
    publisher_id     INTEGER,
    status           INTEGER DEFAULT 0,
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sv_volunteer_signup (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    activity_id    INTEGER NOT NULL,
    volunteer_id   INTEGER NOT NULL,
    signup_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    status         INTEGER DEFAULT 0,
    checkin_time   DATETIME,
    checkout_time  DATETIME,
    hours          REAL DEFAULT 0,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sv_service_provider (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    org_id         INTEGER,
    user_id        INTEGER,
    provider_name  VARCHAR(128) NOT NULL,
    service_types  VARCHAR(256),
    contact_person VARCHAR(64),
    contact_phone  VARCHAR(20),
    service_area   VARCHAR(256),
    rating         REAL DEFAULT 5.0,
    total_orders   INTEGER DEFAULT 0,
    status         INTEGER DEFAULT 0,
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sv_service_order (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    order_no         VARCHAR(32) UNIQUE,
    applicant_id     INTEGER NOT NULL,
    provider_id      INTEGER,
    service_type     INTEGER,
    title            VARCHAR(256),
    description      TEXT,
    appointment_time DATETIME,
    address          VARCHAR(256),
    status           INTEGER DEFAULT 0,
    accept_time      DATETIME,
    finish_time      DATETIME,
    proof_images     VARCHAR(1024),
    rating           INTEGER,
    comment          VARCHAR(512),
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS sv_job_posting (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    publisher_id     INTEGER,
    community_org_id INTEGER,
    title            VARCHAR(256),
    company          VARCHAR(128),
    salary_range     VARCHAR(64),
    requirements     TEXT,
    description      TEXT,
    headcount        INTEGER,
    deadline         DATE,
    status           INTEGER DEFAULT 0,
    create_by        INTEGER,
    create_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by        INTEGER,
    update_time      DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag         INTEGER DEFAULT 0
);

-- ========== 6. NT 消息通知域 ==========

CREATE TABLE IF NOT EXISTS nt_announcement (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    title             VARCHAR(256) NOT NULL,
    content           TEXT,
    announcement_type INTEGER,
    target_scope      VARCHAR(32),
    target_ids        VARCHAR(512),
    publisher_id      INTEGER,
    publish_time      DATETIME,
    is_top            INTEGER DEFAULT 0,
    read_count        INTEGER DEFAULT 0,
    status            INTEGER DEFAULT 0,
    create_by         INTEGER,
    create_time       DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by         INTEGER,
    update_time       DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag          INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS nt_notification (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id           INTEGER NOT NULL,
    title             VARCHAR(256),
    content           VARCHAR(1024),
    notification_type INTEGER,
    biz_type          VARCHAR(64),
    biz_id            INTEGER,
    is_read           INTEGER DEFAULT 0,
    read_time         DATETIME,
    create_time       DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ========== 7. OC 业委会域 ==========

CREATE TABLE IF NOT EXISTS oc_topic (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    committee_id  INTEGER,
    estate_id     INTEGER,
    title         VARCHAR(256) NOT NULL,
    content       TEXT,
    topic_type    INTEGER,
    publisher_id  INTEGER,
    publish_time  DATETIME,
    need_vote     INTEGER DEFAULT 0,
    vote_start    DATETIME,
    vote_end      DATETIME,
    vote_result   INTEGER DEFAULT 0,
    status        INTEGER DEFAULT 0,
    create_by     INTEGER,
    create_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by     INTEGER,
    update_time   DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag      INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS oc_vote (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    topic_id  INTEGER NOT NULL,
    voter_id  INTEGER NOT NULL,
    choice    INTEGER NOT NULL,
    vote_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS oc_public_income (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id       INTEGER,
    period          VARCHAR(32),
    income_amount   REAL DEFAULT 0,
    expense_amount  REAL DEFAULT 0,
    balance         REAL DEFAULT 0,
    detail          TEXT,
    publisher_id    INTEGER,
    publish_time    DATETIME,
    status          INTEGER DEFAULT 0,
    create_by       INTEGER,
    create_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by       INTEGER,
    update_time     DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag        INTEGER DEFAULT 0
);

-- ========== 8. EV 评价域 ==========

CREATE TABLE IF NOT EXISTS ev_evaluation (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    biz_type     VARCHAR(32),
    biz_id       INTEGER,
    evaluator_id INTEGER NOT NULL,
    rating       INTEGER,
    content      VARCHAR(512),
    reply        VARCHAR(512),
    reply_by     INTEGER,
    reply_time   DATETIME,
    eval_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    create_by    INTEGER,
    create_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by    INTEGER,
    update_time  DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag     INTEGER DEFAULT 0
);

-- ========== 9. AI 智能助手域 ==========

CREATE TABLE IF NOT EXISTS ai_knowledge (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    category  VARCHAR(64),
    question  VARCHAR(512) NOT NULL,
    answer    TEXT NOT NULL,
    keywords  VARCHAR(256),
    priority  INTEGER DEFAULT 0,
    hit_count INTEGER DEFAULT 0,
    status    INTEGER DEFAULT 0,
    create_by INTEGER,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by INTEGER,
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag  INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS ai_chat_log (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id            INTEGER,
    session_id         VARCHAR(64),
    role               VARCHAR(16),
    content            TEXT,
    intent             VARCHAR(64),
    matched_knowledge_id INTEGER,
    is_useful          INTEGER,
    create_time        DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ========== 10. KF 考核域 ==========

CREATE TABLE IF NOT EXISTS kf_assessment_config (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    indicator_name     VARCHAR(128),
    indicator_type     INTEGER,
    target_value       REAL,
    weight             REAL,
    assessment_period  VARCHAR(16),
    status             INTEGER DEFAULT 0,
    create_by          INTEGER,
    create_time        DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by          INTEGER,
    update_time        DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag           INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS kf_assessment_result (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    config_id      INTEGER,
    target_org_id  INTEGER,
    target_user_id INTEGER,
    period         VARCHAR(32),
    actual_value   REAL,
    score          REAL,
    rank           INTEGER,
    assess_time    DATETIME,
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS cm_visitor (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id      INTEGER,
    visitor_name   VARCHAR(64),
    phone          VARCHAR(20),
    id_card        VARCHAR(18),
    host_name      VARCHAR(64),
    host_phone     VARCHAR(20),
    host_house_id  INTEGER,
    purpose        VARCHAR(128),
    arrive_time    DATETIME,
    leave_time     DATETIME,
    visitor_count  INTEGER DEFAULT 1,
    status         INTEGER DEFAULT 0,  -- 0=在访, 1=已离开
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS pm_bill (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    bill_no        VARCHAR(32),
    estate_id      INTEGER,
    house_id       INTEGER,
    resident_id    INTEGER,
    bill_type      INTEGER,           -- 1=物业费, 2=水费, 3=电费, 4=停车费, 5=综合
    amount         REAL,
    period         VARCHAR(16),        -- 账期, 如 2026-06
    due_date       DATETIME,
    pay_time       DATETIME,
    pay_method     INTEGER,           -- 1=现金, 2=微信, 3=支付宝, 4=银行转账
    status         INTEGER DEFAULT 0, -- 0=待缴费, 1=已缴费, 2=逾期
    remark         VARCHAR(256),
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS pm_monthly_card (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id      INTEGER,
    plate_no       VARCHAR(16),
    owner_name     VARCHAR(64),
    owner_phone    VARCHAR(20),
    space_id       INTEGER,
    card_type      INTEGER DEFAULT 0, -- 0=月卡, 1=季卡, 2=年卡
    start_date     DATE,
    end_date       DATE,
    fee            REAL,
    status         INTEGER DEFAULT 0, -- 0=已过期, 1=有效, 2=待续费
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS ge_opinion (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    estate_id      INTEGER,
    resident_id    INTEGER,
    category       VARCHAR(32),        -- 环境/设施/安全/服务/其他
    title          VARCHAR(128),
    content        TEXT,
    contact_phone  VARCHAR(20),
    reply_content  TEXT,
    reply_by       INTEGER,
    reply_time     DATETIME,
    status         INTEGER DEFAULT 0,  -- 0=待处理, 1=已回复, 2=已采纳
    is_anonymous   INTEGER DEFAULT 0,
    create_by      INTEGER,
    create_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    update_by      INTEGER,
    update_time    DATETIME DEFAULT CURRENT_TIMESTAMP,
    del_flag       INTEGER DEFAULT 0
);

-- ========== 索引 ==========

CREATE INDEX IF NOT EXISTS idx_sys_user_phone ON sys_user(phone);
CREATE INDEX IF NOT EXISTS idx_sys_user_status ON sys_user(status);
CREATE INDEX IF NOT EXISTS idx_sys_org_parent ON sys_org(parent_id);
CREATE INDEX IF NOT EXISTS idx_sys_org_type ON sys_org(org_type);
CREATE INDEX IF NOT EXISTS idx_cm_building_estate ON cm_building(estate_id);
CREATE INDEX IF NOT EXISTS idx_cm_unit_building ON cm_unit(building_id);
CREATE INDEX IF NOT EXISTS idx_cm_house_estate ON cm_house(estate_id);
CREATE INDEX IF NOT EXISTS idx_cm_house_unit ON cm_house(unit_id);
CREATE INDEX IF NOT EXISTS idx_cm_resident_user ON cm_resident(user_id);
CREATE INDEX IF NOT EXISTS idx_cm_house_resident_house ON cm_house_resident(house_id);
CREATE INDEX IF NOT EXISTS idx_cm_house_resident_resident ON cm_house_resident(resident_id);
CREATE INDEX IF NOT EXISTS idx_cm_grid_community ON cm_grid(community_org_id);
CREATE INDEX IF NOT EXISTS idx_wo_order_estate ON wo_work_order(estate_id);
CREATE INDEX IF NOT EXISTS idx_wo_order_reporter ON wo_work_order(reporter_id);
CREATE INDEX IF NOT EXISTS idx_wo_order_status ON wo_work_order(status);
CREATE INDEX IF NOT EXISTS idx_wo_order_create ON wo_work_order(create_time);
CREATE INDEX IF NOT EXISTS idx_ge_event_grid ON ge_event(grid_id);
CREATE INDEX IF NOT EXISTS idx_ge_event_status ON ge_event(status);
CREATE INDEX IF NOT EXISTS idx_ge_event_create ON ge_event(create_time);
CREATE INDEX IF NOT EXISTS idx_ge_event_flow_event ON ge_event_flow(event_id);
CREATE INDEX IF NOT EXISTS idx_sv_activity_community ON sv_volunteer_activity(community_org_id);
CREATE INDEX IF NOT EXISTS idx_sv_order_applicant ON sv_service_order(applicant_id);
CREATE INDEX IF NOT EXISTS idx_sv_order_provider ON sv_service_order(provider_id);
CREATE INDEX IF NOT EXISTS idx_nt_notification_user ON nt_notification(user_id);
CREATE INDEX IF NOT EXISTS idx_nt_notification_read ON nt_notification(is_read);
CREATE INDEX IF NOT EXISTS idx_sys_oplog_user ON sys_operation_log(user_id);
CREATE INDEX IF NOT EXISTS idx_sys_oplog_time ON sys_operation_log(operation_time);
