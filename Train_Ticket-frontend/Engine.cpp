#include "Engine.h"

#ifdef my_debug

extern ofstream core_file ;

extern bool stop_core ;

#endif

extern string front_end_answer ;

bool ride::operator<(const ride &other) const
{
    if ( real_type == money ){
        if ( money_cost == other.money_cost ){
            return trainID < other.trainID ;
        }else return money_cost < other.money_cost ;
    }
    else if ( time_cost == other.time_cost ){
        return trainID < other.trainID ;
    }else return time_cost < other.time_cost ;
}

void ride::print_ride()
{
    date set_off_date = ride_purchase_day , arrive_in_date = ride_purchase_day + time_cost ;
//    cout << trainID << " " << from_location << " " ;
    front_end_answer.append( trainID + string(" ") + from_location + string(" ") ) ;
    set_off_date.print_date() ;
//    cout << " -> " << to_location << " " ;
    front_end_answer.append(string(" -> ") + to_location + string(" ") ) ;
    arrive_in_date.print_date() ;
//    cout << " " << money_cost << " " << ride_max_available_ticket << endl ;
    front_end_answer.append(string(" ") + IntToSTRING(money_cost) +  string(" ") + IntToSTRING(ride_max_available_ticket) + "\n" ) ;
}

ride::ride(train &temp_train, int temp_location_1, int temp_location_2, date purchase_day)
{
    location_1 = temp_location_1 ;
    location_2 = temp_location_2 ;
    trainID = temp_train.trainID ;
    from_location = temp_train.all_station[temp_location_1] ;
    to_location = temp_train.all_station[temp_location_2] ;
    money_cost = temp_train.get_price(location_1,location_2) ;
    time_cost = temp_train.get_time(location_1,location_2) ;
    ride_purchase_day = purchase_day ;
    ride_purchase_day.get_other_time(temp_train.all_set_off[location_1]) ;
    ride_max_available_ticket = temp_train.get_max_available_ticket(purchase_day,location_1,location_2) ;
}

void ride::ride_modify( train &temp_train , int temp_location_1 , int temp_location_2 , date purchase_day )
{
    ride temp_ride(temp_train,temp_location_1,temp_location_2,purchase_day) ;
    *this = temp_ride ;
}


my_system::my_system() : user_tree(string(USER_FILE)) , train_tree(string(TRAIN_FILE)) , user_deal_tree(string(DEAL_FILE)) ,
                         location_train_tree(string(LOCATION_FILE)) , waiting_tree(string(WAITING_LIST_FILE)) , real_train_file(REAL_TRAIN_FILE,ios::in|ios::out|ios::binary) // ????????????????????????
{
    if ( user_tree.size() == 0 ){
        ofstream temp_file(REAL_TRAIN_FILE,ios::binary) ;
        temp_file.close() ;
        real_train_file.clear() ;
        real_train_file.open(REAL_TRAIN_FILE,ios::in|ios::out|ios::binary) ;
    }
}

int my_system::train_insert( train &temp_train ) // todo ?????????????????????????????? clear
{
    real_train_file.seekp(0,ios::end) ;
    int temp_pos = real_train_file.tellp() ;
    real_train_file.write(reinterpret_cast<char*>(&temp_train),sizeof(train)) ;
    return temp_pos ;
}

void my_system::read_train( int train_pos , train &temp_train )
{
    real_train_file.seekg(train_pos,ios::beg) ;
    real_train_file.read(reinterpret_cast<char*>(&temp_train),sizeof(train)) ;
    real_train_file.clear() ;
}

bool my_system::check_priority(string &c_user_name, user &u_user)
{
    string temp_name(u_user.user_name) ;
    if ( log_in_user[c_user_name] > u_user.privilege || c_user_name == temp_name ) return true ;
    return false ;
}

bool my_system::check_login(string &c_user_name){ return log_in_user.count(c_user_name) ; }

void my_system::success() {
//    cout << 0 << endl ;
    front_end_answer.append("0\n") ;
}

void my_system::fail( string err_inf  ) {
//    cout << -1 << endl ;
    front_end_answer.append("-1\n") ;
#ifdef my_debug
    cerr << command_stream.str() << endl ;
    if ( !err_inf.empty() ){
        cerr << err_inf << endl ;
    }
#endif
}

void my_system::user_update(user &u_user)
{
    IndexKey user_key(u_user) ;
//    if( !user_tree.erase(user_key,u_user ) ) {}
////        cerr << "no such user" << endl  ;
//    user_tree.insert(user_key,u_user) ;
    user &real_user = user_tree.update(user_key,u_user) ;
    real_user = u_user ; // ??????????????????
}

void my_system::train_update( int train_pos , train &t_train )
{
//    IndexKey train_key(t_train) ;
//    train_tree.erase(train_key,t_train) ;
//    train_tree.insert(train_key,t_train) ;
    real_train_file.seekp(train_pos,ios::beg) ;
    real_train_file.write(reinterpret_cast<char*>(&t_train),sizeof(train)) ;
}

void my_system::deal_update(ticket_deal &t_deal)
{
    IndexKey user_deal_key ;
    strcpy( user_deal_key.real_key , t_deal.user_name ) ;
    t_deal.modify_waiting(false) ;
//    user_deal_tree.erase(user_deal_key,t_deal) ;
//    user_deal_tree.insert(user_deal_key,t_deal) ;
    ticket_deal &real_deal = user_deal_tree.update(user_deal_key,t_deal) ;
    real_deal = t_deal ;
}

void my_system::waiting_update(ticket_deal &t_deal)
{
    IndexKey waiting_deal_key ;
    strcpy( waiting_deal_key.real_key , t_deal.trainID ) ;
    t_deal.modify_waiting(true) ;
    if ( t_deal.deal_status == pending )
        waiting_tree.insert(waiting_deal_key,t_deal) ;
    else waiting_tree.erase(waiting_deal_key,t_deal) ;
}

void my_system::process_command(string &all_command)
{
    string real_command ;
    command_stream.str("") ;
    command_stream.clear() ;
    command_stream << all_command ;
    command_stream >> real_command ;
    if ( real_command == "add_user" ) { add_user() ; }
    else if ( real_command == "login" ) { login() ; }
    else if ( real_command == "logout" ) { logout() ; }
    else if ( real_command == "query_profile" ) { query_profile() ; }
    else if ( real_command == "modify_profile" ) { modify_profile() ; }
    else if ( real_command == "add_train" ) { add_train() ; }
    else if ( real_command == "release_train" ) { release_train() ; }
    else if ( real_command == "query_train" ) { query_train() ; }
    else if ( real_command == "delete_train" ) { delete_train() ; }
    else if ( real_command == "query_ticket" ) { query_ticket() ; }
    else if ( real_command == "query_transfer" ) { query_transfer() ; }
    else if ( real_command == "buy_ticket" ) { buy_ticket() ; }
    else if ( real_command == "query_order" ) { query_order() ; }
    else if ( real_command == "refund_ticket" ) { refund_ticket() ; }
    else if ( real_command == "exit" ) { EXIT() ; }
    else if ( real_command == "clean" ) { clean() ; }
}

bool my_system::no_repeated_user(user &u_user)
{
    IndexKey user_key(u_user) ;
    vector<user> ans_vec ;
    user_tree.find(user_key,ans_vec) ;
    if ( ans_vec.empty() ) return true ;
//    cerr << "repeated user" << endl ;
    return false ;
}

void my_system::add_user()
{
    para temp_para(command_stream) ;
    user temp_user(temp_para) ;
    if ( !user_tree.empty() && (!check_login(temp_para.c) || !check_priority(temp_para.c,temp_user) || !no_repeated_user(temp_user)) ){ fail() ; return ; }
    if ( user_tree.empty() ) temp_user.privilege = 10 ;
    IndexKey user_key(temp_para.u) ;
    user_tree.insert(user_key,temp_user) ;
    success() ;
}

void my_system::login()
{
    para temp_para(command_stream) ;
    IndexKey user_key(temp_para.u) ;
    if ( log_in_user.count(temp_para.u) ){ fail() ; return ; } // ????????????????????????
    vector<user> ans_vec ;
    user_tree.find(user_key,ans_vec) ;
    if ( ans_vec.empty() || !ans_vec[0].right_password(temp_para.p) ) { fail(); return ; }
    log_in_user[temp_para.u] = ans_vec[0].privilege ;
    success() ;
}

void my_system::logout()
{
    para temp_para( command_stream ) ;
    if ( !log_in_user.count(temp_para.u) ) { fail(); return ; }
    log_in_user.erase(temp_para.u) ;
    success() ;
}

void my_system::query_profile()
{
    para temp_para( command_stream ) ;
    if ( !check_login(temp_para.c) ) { fail(); return ; }
    IndexKey user_key(temp_para.u) ;
    vector<user> ans_vec ;
    user_tree.find(user_key,ans_vec) ;
    if ( ans_vec.empty() || !check_priority(temp_para.c,ans_vec[0]) ) { fail(); return ; }
    ans_vec[0].print_user() ;
}

void my_system::modify_profile()
{
    para temp_para(command_stream) ;
    if ( !check_login(temp_para.c) ) { fail(); return ; }
    IndexKey user_key(temp_para.u) ;
    vector<user> ans_vec ;
    user_tree.find(user_key,ans_vec) ;
    if ( ans_vec.empty() || !check_priority(temp_para.c,ans_vec[0]) ) { fail(); return ; }
    stringstream change_stream ;
    if ( !temp_para.p.empty() )
    { change_stream << temp_para.p ; change_stream >> ans_vec[0].password ; change_stream.str("") ; change_stream.clear() ; }
    if ( !temp_para.n.empty() )
    { change_stream << temp_para.n ; change_stream >> ans_vec[0].chinese_name ; change_stream.str("") ; change_stream.clear() ; }
    if ( !temp_para.m.empty() )
    { change_stream << temp_para.m ; change_stream >> ans_vec[0].mailAddr ; change_stream.str("") ; change_stream.clear() ; }
    if ( !temp_para.g.empty() )
    { change_stream << temp_para.g ; change_stream >> ans_vec[0].privilege ; change_stream.str("") ; change_stream.clear() ; }
    if ( ans_vec[0].privilege >= log_in_user[temp_para.c] && !temp_para.g.empty() ) { fail(); return ; }
    if ( check_login(temp_para.u) ){
        log_in_user[temp_para.u] = ans_vec[0].privilege ;
    }
    user_update(ans_vec[0]) ;
    ans_vec[0].print_user() ;
}

void my_system::add_train()
{
    para temp_para(command_stream) ;
    IndexKey train_key(temp_para.i) ;
    vector<int> ans_vec ;
    train_tree.find(train_key,ans_vec) ;
    if ( !ans_vec.empty() ) { fail() ; return ; }
    train temp_train(temp_para) ;
    train_tree.insert(train_key, train_insert(temp_train)) ; // key -> train_pos
    success() ;
}

void my_system::release_train() // todo ??? location_train_key -> int
{
    para temp_para(command_stream) ;
    IndexKey train_key(temp_para.i) ;
    vector<int> ans_vec ;
    train_tree.find(train_key,ans_vec) ;
    if ( ans_vec.empty() ) { fail() ; return ; }
    train temp_train ;
    read_train(ans_vec[0],temp_train) ;
    if ( temp_train.is_released() ){ fail() ; return ; }
    temp_train.release_train() ; // ????????? release ??????
    for ( int i = 1 ; i <= temp_train.station_num ; i++ ){
        string temp_location(temp_train.all_station[i]) ;
        IndexKey location_key(temp_location) ;
        location_train_tree.insert(location_key,ans_vec[0]) ;
    }
//    if ( !train_tree.erase(train_key,ans_vec[0]) ){
//        cerr << "release erase fail" << endl ;
//    } ;
//    train_tree.insert(train_key,ans_vec[0]) ;
    train_update(ans_vec[0],temp_train) ;
    success() ;
}

void my_system::query_train()
{
    para temp_para(command_stream) ;
    IndexKey train_key(temp_para.i) ;
    vector<int> ans_vec ;
    train_tree.find(train_key,ans_vec) ;
    date temp_date(temp_para.d) ;
    train temp_train ;
    if ( ans_vec.empty() ){ fail() ; return ; }
    read_train(ans_vec[0],temp_train) ;
    if ( !temp_train.in_sale(temp_date,1) ) { fail() ; return ; }
    temp_train.print_train(date(temp_date)) ;
}

void my_system::delete_train()
{
    para temp_para(command_stream) ;
    IndexKey train_key(temp_para.i) ;
    vector<int> ans_vec ;
    train_tree.find(train_key,ans_vec) ;
    if ( ans_vec.empty() ) { fail() ; return ; }
    train temp_train ;
    read_train(ans_vec[0],temp_train) ;
    if ( temp_train.is_released() ) { fail() ; return ; }
    train_tree.erase(train_key,ans_vec[0]) ;
    success() ;
}


void my_system::make_ride( string &from_location , string &to_location , vector<ride> &ans_vec , vector<int> &all_train_key , date purchase_day ) // todo ????????????
{
//    vector<train> train_vec ;
    train temp_train ;
    for ( int i = 0 ; i < all_train_key.size() ; i++ ){
        read_train(all_train_key[i],temp_train) ;
        int int_location_1 = temp_train.get_location(from_location) , int_location_2 = temp_train.get_location(to_location) ;
        if ( int_location_1 >= int_location_2 ) continue ;
        if ( !temp_train.in_sale(purchase_day,int_location_1) ) continue ;
        ride temp_ride(temp_train,int_location_1,int_location_2,purchase_day) ;
        ans_vec.push_back(temp_ride) ;
    }
}

void my_system::query_ticket() // todo ????????? train_key ?????? pos
{
    para temp_para(command_stream) ;
    IndexKey from_location_key(temp_para.s) , to_location_key(temp_para.t) ;
//    vector<train> from_train , to_train , temp_vec ;
    vector<int> from_train_key , to_train_key , all_train_key ;
    date purchase_day(temp_para.d) ; // todo  ???????????????????????? sale_begin ?????? ??? ????????????????????? sale_begin ??????
    location_train_tree.find(from_location_key,from_train_key) ;
    location_train_tree.find(to_location_key,to_train_key) ; // todo ????????????????????? Key

    for ( int i = 0 ; i < from_train_key.size() ; i++ ){
        for ( int j = 0 ; j < to_train_key.size() ; j++ ){
            if ( from_train_key[i] == to_train_key[j] ){
                all_train_key.push_back(from_train_key[i]) ;
                break ;
            }
        }
    }

// todo ??????????????? ?????????????????????
    vector<ride> available_ride ; // todo ?????????
    make_ride(temp_para.s,temp_para.t,available_ride,all_train_key,purchase_day) ;
    if ( temp_para.p == "cost" )
        for ( auto it = available_ride.begin() ; it != available_ride.end() ; it++){ it->real_type = money ; }
    sort(available_ride.begin(),available_ride.end(),less<ride>()) ;
    if ( available_ride.empty() ) { success() ; return ; } // ??????
//    cout << available_ride.size() << endl ;
    front_end_answer.append(IntToSTRING(available_ride.size()) + string("\n")) ;
    for ( int i = 0 ; i < available_ride.size() ; i++ ){
        // todo trainID ?????????
        available_ride[i].print_ride() ;
    }
}

void my_system::query_transfer()
{
    para temp_para(command_stream) ;
    IndexKey from_location_key(temp_para.s) ;
//    vector<train> train_vec_1 , train_vec_2 ;
    train temp_train_1 , temp_train_2 ;
    vector<int> train_key_1 , train_key_2 ;
    ride ans_ride_1 , ans_ride_2 , temp_ride_1 , temp_ride_2 ;
    int ans_cost = MAX_MONEY_COST ; // ?????????
    date ans_purchase_1 , ans_purchase_2 ;
    if ( temp_para.p == "cost" ) { ans_ride_1.real_type = ans_ride_2.real_type = temp_ride_1.real_type = temp_ride_2.real_type = money ; } // todo ???????????????????????????
    location_train_tree.find(from_location_key,train_key_1) ;
    date purchase_day(temp_para.d) ;
    string mid_location ;
    for ( int i = 0 ; i < train_key_1.size() ; i++ ){
//        train_vec_1.clear() ;
//        train_tree.find(train_key_1[i],train_vec_1) ;
        read_train(train_key_1[i],temp_train_1) ;
        int start_point = temp_train_1.get_location(temp_para.s) ;
        if ( !temp_train_1.in_sale(purchase_day,start_point) ) continue ; // todo ?????????????????????
        purchase_day.get_other_time(temp_train_1.all_set_off[start_point]) ;
        for ( int j = start_point + 1 ; j <= temp_train_1.station_num ; j++ ){
            mid_location = temp_train_1.all_station[j] ;
            int first_mid_point = j ; // ????????????????????????
            IndexKey mid_location_key(mid_location) ;
            train_key_2.clear() ;
            location_train_tree.find(mid_location_key,train_key_2) ;
            for ( int k = 0 ; k < train_key_2.size() ; k++ ){ // todo ???????????????????????????????????????
                if ( train_key_2[k] == train_key_1[i] ) continue ; // ???????????????
//                train_tree.find(train_key_2[k],train_vec_2) ;
                read_train(train_key_2[k],temp_train_2) ;
                int second_mid_point = temp_train_2.get_location(mid_location) , end_point = temp_train_2.get_location(temp_para.t) ;
                if ( second_mid_point >= end_point ){
                    continue ; // ??????????????????
                }
                // ?????????
                date mid_arrive_in_day = purchase_day + temp_train_1.get_time(start_point,first_mid_point) ;
                date mid_set_off_day = mid_arrive_in_day ;
                if ( !temp_train_2.can_take_in_time(mid_set_off_day,second_mid_point ) ) { // todo ???????????????????????????
                    continue ;
                }
                temp_ride_1.ride_modify(temp_train_1,start_point,first_mid_point,purchase_day) ;
                temp_ride_2.ride_modify(temp_train_2,second_mid_point,end_point,mid_set_off_day) ;
                if ( temp_para.p != "cost" ){
                    int temp_ans = mid_set_off_day - purchase_day + temp_ride_2.time_cost ;
                    if ( temp_ans < ans_cost || ( temp_ans == ans_cost && temp_ride_1.time_cost < ans_ride_1.time_cost ) ){
                        ans_ride_1 = temp_ride_1 ;
                        ans_ride_2 = temp_ride_2 ;
                        ans_cost = temp_ans ;
                    }
                }else{
                    int temp_ans = temp_ride_1.money_cost + temp_ride_2.money_cost ;
                    if ( temp_ans < ans_cost || ( temp_ans == ans_cost && temp_ride_1.time_cost < ans_ride_1.time_cost ) ){
                        ans_ride_1 = temp_ride_1 ;
                        ans_ride_2 = temp_ride_2 ;
                        ans_cost = temp_ans ;
                    }
                }
            }
        }
    }
    if ( ans_cost == MAX_MONEY_COST ) { success() ; return ; }
    ans_ride_1.print_ride() ;
    ans_ride_2.print_ride() ;
}

void my_system::buy_ticket()
{
    para temp_para(command_stream) ;
    if ( !check_login(temp_para.u) ) { fail() ; return ; }
    IndexKey train_key(temp_para.i) , user_key(temp_para.u) ;
    vector<int> ans_vec  ; vector<user> user_vec ;
    train temp_train ;
    train_tree.find(train_key,ans_vec) ;
    user_tree.find(user_key,user_vec) ;
    date purchase_day(temp_para.d) ; // todo ??? location ??????????????????
    if ( ans_vec.empty() ) { fail("no such train") ; return ; }
    read_train(ans_vec[0],temp_train) ;
    int location_1 = temp_train.get_location(temp_para.f) , location_2 = temp_train.get_location(temp_para.t) ;
    if ( !temp_train.is_released() || location_1 == -1 || location_2 == -1 || location_1 >= location_2 || !temp_train.in_sale(purchase_day,location_1) ) { fail("invalid train") ; return ; } // todo Mingwin ????????????????????????
    ticket_deal temp_deal(temp_para) ;
    int max_available_tickets = temp_train.get_max_available_ticket(purchase_day,location_1,location_2) ;
    if ( temp_deal.ticket_num > temp_train.seat_num || ( max_available_tickets < temp_deal.ticket_num && temp_para.q != "true" ) ) { fail() ; return ; }
    temp_deal.ticket_modify(user_vec[0],temp_train,location_1,location_2,purchase_day) ;
    if ( max_available_tickets >= temp_deal.ticket_num ){
        temp_train.ticket_decrease(purchase_day,location_1,location_2,temp_deal.ticket_num) ;
        temp_deal.deal_status = succeed ;
//        cout << ( long long ) temp_deal.price * ( long long ) temp_deal.ticket_num << endl ;
        front_end_answer.append(IntToSTRING(temp_deal.price*temp_deal.ticket_num) + string("\n") ) ;
    }else{
        temp_deal.deal_status = pending ;
        temp_deal.isWaiting = true ;
//        cout << "queue" << endl ;
        front_end_answer.append("queue\n") ;
        waiting_tree.insert(train_key,temp_deal) ;
    }
    user_vec[0].change_deal() ;
    temp_train.change_waiting_length(1) ;
    user_update(user_vec[0]) ;
    train_update(ans_vec[0],temp_train) ;
    temp_deal.isWaiting = false ;
    user_deal_tree.insert(user_key,temp_deal) ;
#ifdef my_debug

    if ( !stop_core && temp_para.i == "LeavesofGrass" && ( temp_para.d == "06-18" || temp_para.d == "06-19" ) ){
        core_file << "buy_ticket -u I_am_the_admin -i LeavesofGrass -d " << temp_para.d << " -n " << temp_para.n << " -f " << temp_para.f << " -t " << temp_para.t ;
        if ( !temp_para.q.empty() ) core_file << " -q " << temp_para.q ;
        core_file << endl ;
        if ( command_stream.str() == "buy_ticket -u Matoimaru -i LeavesofGrass -d 06-19 -n 3691 -f ????????????????????? -t ?????????????????? -q true" ) stop_core = true ;
    }

#endif
}

void my_system::query_order()
{
    para temp_para(command_stream) ;
    if ( !check_login(temp_para.u) ) { fail() ; return ; } // todo ??????????????????????????? buy ??????????????? update_deal ????????????
    IndexKey user_key(temp_para.u) ;
    vector<ticket_deal> ans_vec ;
    user_deal_tree.find(user_key,ans_vec) ;
//    cout << ans_vec.size() << endl ;
    front_end_answer.append(IntToSTRING(ans_vec.size()) + string("\n")) ;
    sort(ans_vec.begin(),ans_vec.end(),less<ticket_deal>()) ;
    for ( int i = ans_vec.size() - 1 ; i >= 0 ; i-- ){
        ans_vec[i].print_deal() ;
    }
}

void my_system::refund_ticket() // todo line 265 refund ?????? finish
{
    para temp_para(command_stream) ;
    if ( !check_login(temp_para.u) ) { fail("not log in") ; return ; } // ????????????
    int target_deal = 1 ;
    if ( !temp_para.n.empty() ){  // ?????? n ??? ???????????? i ?????????
        stringstream change_stream ;
        change_stream << temp_para.n ;
        change_stream >> target_deal ;
    }
    IndexKey user_key(temp_para.u) ;
    vector<ticket_deal> ans_vec , waiting_vec ;
    user_deal_tree.find(user_key,ans_vec) ;
    sort(ans_vec.begin(),ans_vec.end(),less<ticket_deal>()) ;
    if ( target_deal > ans_vec.size() ) { fail("no so many deals") ; return ; } // ?????????????????????
    ticket_deal temp_deal = ans_vec[ans_vec.size()-target_deal] ;
    if ( temp_deal.deal_status == refunded ) { fail() ; return ; } // ????????????
    string train_str = temp_deal.trainID , from_location = temp_deal.from_location , to_location = temp_deal.to_location ; // ?????? waiting_list \ train \ deal
    IndexKey train_key(train_str) ;
    if ( temp_deal.deal_status == pending ){ // todo ??? queue ??????
        temp_deal.deal_status = refunded ;
        deal_update(temp_deal) ;
        temp_deal.isWaiting = true ; // ?????????????????????
        waiting_tree.erase(train_key,temp_deal) ;
        success() ;
        return ;
    }
    vector<int> train_vec ;
    train_tree.find(train_key,train_vec) ;
    train temp_train ;
    read_train(train_vec[0],temp_train) ;
    temp_train.ticket_increase(temp_deal.departure_time,temp_train.get_location(from_location),temp_train.get_location(to_location),temp_deal.ticket_num) ; // ??????????????????
    waiting_tree.find(train_key,waiting_vec) ;
    sort(waiting_vec.begin(),waiting_vec.end(),less<ticket_deal>()) ;
    for ( int i = 0 ; i < waiting_vec.size() ; i++ ){ // todo ?????? waiting_vec ????????????????????? ?
        ticket_deal waiting_deal = waiting_vec[i] ;
        from_location = waiting_vec[i].from_location ;
        to_location = waiting_vec[i].to_location ;
        int location_1 = temp_train.get_location(from_location) , location_2 = temp_train.get_location(to_location) ; // todo ????????????
        int available_tickets = temp_train.get_max_available_ticket(waiting_deal.departure_time,location_1,location_2) ;
        if ( available_tickets >= waiting_deal.ticket_num ){
            temp_train.ticket_decrease(waiting_deal.departure_time,location_1,location_2,waiting_deal.ticket_num) ; // ?????????????????????
            waiting_tree.erase(train_key,waiting_deal) ; // ????????????????????????
            waiting_deal.deal_status = succeed ;
            waiting_deal.isWaiting = false ;
            deal_update(waiting_deal) ;
        }
    }
    train_update(train_vec[0],temp_train) ; // ??????????????????
    temp_deal.deal_status = refunded ;
    deal_update(temp_deal) ;
    success() ;
}

void my_system::EXIT() {
    cout << "bye" << endl ;
    front_end_answer.append("bye\n") ;
    exit(0) ;
}

void my_system::clean()
{
    user_tree.clear() ;
    train_tree.clear() ;
    user_deal_tree.clear() ;
    location_train_tree.clear() ;
    waiting_tree.clear() ;
    log_in_user.clear() ;
    success() ;
}








